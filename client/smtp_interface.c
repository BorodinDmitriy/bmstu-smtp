#include "smtp_interface.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//
int getMXrecord(struct FileDesc *connection);
int getCommandStatus(char *message);
int verifyServerDomain(char *message, char *domain);
int setLetterMetaData(struct letter_info *info, char *filepath);

//============//
//  HANDLERS  //
//============//

int handlePrepareSocketConnection(struct FileDesc *connection);
int handleGreeting(struct FileDesc *connection);
int handleSendEHLO(struct FileDesc *connection);
int handleResponseOfEHLO(struct FileDesc *connection);
int handleSendMailFrom(struct FileDesc *connection);
int handleResponseOfMailFrom(struct FileDesc *connection);
int handleSendRCPTto(struct FileDesc *connection);
int handleResponseOfRCPTto(struct FileDesc *connection);
int handleSendDATA(struct FileDesc *connection);
int handleResponseOfDATA(struct FileDesc *connection);
int handleSendLetter(struct FileDesc *connection);
int handleResponseOfLetter(struct FileDesc *connection);
int handleSendQUIT(struct FileDesc *connection);
int handleResponseOfQUIT(struct FileDesc *connection);
int handleDisponsing(struct FileDesc *connection);
int handleSmtpError(struct FileDesc *connection);
int handleSolvableMistake(struct FileDesc *connection);
int handleResponseOfRSET(struct FileDesc *connection);

//==========================//
//      PUBLIC METHODS      //
//==========================//

int SMTP_Control(struct FileDesc *socket_connection)
{
    int state = 0;
    while (state == 0)
    {
        // printf("socket_connection: %d\n", socket_connection->current_state);
        switch (socket_connection->current_state)
        {
        case PREPARE_SOCKET_CONNECTION:
            state = handlePrepareSocketConnection(socket_connection);
            break;

        case RECEIVE_SMTP_GREETING:
            state = handleGreeting(socket_connection);
            break;

        case SEND_EHLO:
            state = handleSendEHLO(socket_connection);
            break;

        case RECEIVE_EHLO_RESPONSE:
            state = handleResponseOfEHLO(socket_connection);
            break;

        case SEND_MAIL_FROM:
            state = handleSendMailFrom(socket_connection);
            break;

        case RECEIVE_MAIL_FROM_RESPONSE:
            state = handleResponseOfMailFrom(socket_connection);
            break;

        case SEND_RCPT_TO:
            state = handleSendRCPTto(socket_connection);
            break;

        case RECEIVE_RCPT_TO_RESPONSE:
            state = handleResponseOfRCPTto(socket_connection);
            break;

        case SEND_DATA:
            state = handleSendDATA(socket_connection);
            break;

        case RECEIVE_DATA_RESPONSE:
            state = handleResponseOfDATA(socket_connection);
            break;

        case SEND_LETTER:
            state = handleSendLetter(socket_connection);
            break;

        case RECEIVE_LETTER_RESPONSE:
            state = handleResponseOfLetter(socket_connection);
            break;

        case SEND_QUIT:
            state = handleSendQUIT(socket_connection);
            break;

        case RECEIVE_QUIT_RESPONSE:
            state = handleResponseOfQUIT(socket_connection);
            break;

        case DISPOSING_SOCKET:
            state = handleDisponsing(socket_connection);
            break;

        case SMTP_ERROR:
            state = handleSmtpError(socket_connection);
            break;

        case SEND_RSET:
            state = handleSolvableMistake(socket_connection);
            break;

        case RECEIVE_RSET_RESPONSE:
            state = handleResponseOfRSET(socket_connection);
            break;

        default:
            state = -1;
        }

        //  wait letters by current domain
        if (socket_connection->prev_state == PREPARE_SOCKET_CONNECTION) 
        {
            break;
        }
    }
    return state;
}

//==========================//
//      PRIVATE METHOD      //
//==========================//

//============//
//  HANDLERS  //
//============//

int handlePrepareSocketConnection(struct FileDesc *connection)
{
    int state = 0;

    //  Check states
    if (connection->current_state != PREPARE_SOCKET_CONNECTION || connection->prev_state != NULL_POINTER)
    {
        char message[150];
        memset(message, '\0', 150);
        sprintf(message, "Worker: handlePrepareSocketConnection: unexpected current_state (%d) and prev_state(%d) for socket with domain %s", connection->current_state, connection->prev_state, connection->domain);
        Error(message);
        return -1;
    }

    // State is OK

    //  Resolve MX_RECORD
    state = getMXrecord(connection);

    if (state < 0)
    {
        return -2;
    }

    connection->id = socket(AF_INET, SOCK_STREAM, 0);

    int opt_val = 1;
    socklen_t opt_len = sizeof(opt_val);

    state = setsockopt(connection->id, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, opt_len);
    if (state)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: handlePrepareSocketConnection: Fail to set socket option 'SO_REUSEADDR' for %s domain", connection->domain);
        Error(message);
        return -3;
    }

    state = setsockopt(connection->id, SOL_SOCKET, SO_REUSEPORT, (char *)&opt_val, opt_len);
    if (state)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: handlePrepareSocketConnection: Fail to set socket option 'SO_REUSEPORT' for %s domain", connection->domain);
        Error(message);
        return -4;
    }

    int file_flags = fcntl(connection->id, F_GETFL, 0);
    if (file_flags == -1)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: handlePrepareSocketConnection: Fail to receive socket flags for %s domain", connection->domain);
        Error(message);
        return -5;
    }

    state = fcntl(connection->id, F_SETFL, file_flags | O_NONBLOCK);
    if (state)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: handlePrepareSocketConnection: Fail to set flag 'O_NONBLOCK' for socket by %s domain", connection->domain);
        Error(message);
        return -6;
    }

    connection->addr.sin_family = AF_INET;
    connection->addr.sin_port = htons(SERVER_PORT);

    state = inet_pton(AF_INET, connection->mx_record, &connection->addr.sin_addr);
    if (state <= 0)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: handlePrepareSocketConnection: Fail to inet_pton for %s domain", connection->domain);
        Error(message);
        return -7;
    }

    state = connect(connection->id, (struct sockaddr *)&connection->addr, sizeof(connection->addr));
    if (state < 0 && (state != -1 && (errno != EAGAIN || errno!= EINPROGRESS)))
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: handlePrepareSocketConnection: Fail to inet_pton for %s domain", connection->domain);
        Error(message);
        return -8;
    }

    //  CHANGE CURRENT STATE AND PREV STATE
    connection->current_state = RECEIVE_SMTP_GREETING;
    connection->prev_state = PREPARE_SOCKET_CONNECTION;

    if (state == -1 && (errno == EAGAIN || errno == EINPROGRESS))
    {
        //  Socket is block on connection
        return 1;
    }

    //  All ok.
    return 0;
}

int handleGreeting(struct FileDesc *connection)
{
    //  Check state
    if (connection->current_state != RECEIVE_SMTP_GREETING || connection->prev_state != PREPARE_SOCKET_CONNECTION)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    //  Current state is ok

    char message[BUFFER];
    memset(message, '\0', BUFFER);
    int size;

    size = recv(connection->id, message, BUFFER, NULL);

    if (size == -1)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleGreeting: Fail to receive greeting from server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);

    //  Unexpected status codes
    if (status != 220 && status != 421)
    {
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    if (status == 421)
    {
        connection->current_state = DISPOSING_SOCKET;
        return 0;
    }

    status = verifyServerDomain(message, connection->domain);

    //  Заглушка 
    int d = strncmp("samsung-np530u4c", connection->domain, 11);
    if (d == 0)
    {
        status = 0;
    }

    if (status != 0)
    {
        //  Unexpected domain
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    //  Successfully change state, all actions is ok
    connection->prev_state = connection->current_state;
    connection->current_state = SEND_EHLO;

    return 0;
}

int handleSendEHLO(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != SEND_EHLO || connection->prev_state != RECEIVE_SMTP_GREETING)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }
    int len = strlen(MY_DOMAIN);

    char message[5 + len + 3];

    memset(message, '\0', 8 + len);
    sprintf(message, "EHLO %s\r\n", MY_DOMAIN);

    int size = send(connection->id, message, len + 7, NULL);

    if (size == -1)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleSendEHLO: Fail to send EHLO command to server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    //  Change current state
    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_EHLO_RESPONSE;

    return 0;
}

int handleResponseOfEHLO(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != RECEIVE_EHLO_RESPONSE || connection->prev_state != SEND_EHLO)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;

        return -1;
    }

    char message[BUFFER];
    int size = recv(connection->id, message, BUFFER, NULL);

    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseByEHLO: Fail to receive EHLO response from server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);
    if (status != 250 && status != 421)
    {
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    if (status == 421)
    {
        connection->current_state = DISPOSING_SOCKET;
        return 0;
    }

    //  check message
    int len = strlen(MY_DOMAIN);
    char *pointer = strstr(message, MY_DOMAIN);
    if (!pointer)
    {
        //  In EHLO not founded my name... is error connection
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    //  check CRLF
    if (message[size - 2] != '\r' || message[size - 1] != '\n' || (pointer + len)[0] != '\r' || (((pointer - 1)[0] != ' ') && (pointer - 1)[0] != '-'))
    {
        //  Bad command. Error in CRLF or my domain
        connection->current_state = SMTP_ERROR;
        return -5;
    }

    connection->attempt = 0;

    //  change current state
    connection->prev_state = connection->current_state;
    connection->current_state = SEND_MAIL_FROM;

    return 0;
}

int handleSendMailFrom(struct FileDesc *connection)
{
    //  Check states and transition

    bool first_sending = connection->prev_state == RECEIVE_EHLO_RESPONSE;
    bool repeated_sending = connection->prev_state == RECEIVE_LETTER_RESPONSE;
    bool fail_sending = connection->prev_state == RECEIVE_RSET_RESPONSE;

    if (connection->current_state != SEND_MAIL_FROM || (!first_sending && !repeated_sending && !fail_sending))
    {
        //  Set error state
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;

        return -1;
    }

    int state = setLetterMetaData(&connection->meta_data, connection->task_pool->path);

    if (state != 0)
    {
        char message[250];
        memset(message, '\0', 250);
        sprintf(message, "Worker: handleSendMailFrom: Fail to receive metadata about letter with filepath(%s) by domain (%s)", connection->task_pool->path, connection->domain);
        Error(message);

        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;

        return -2;
    }

    //  send MAIL command

    char message[BUFFER];
    memset(message, '\0', BUFFER);
    sprintf(message, "MAIL FROM:<%s>\r\n", connection->meta_data.from);
    int len = strlen(message);

    state = send(connection->id, message, len, NULL);
    if (state < 0)
    {
        if (state == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleSendMailFrom: Fail to send MAIL FROM command to server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_MAIL_FROM_RESPONSE;

    return 0;
}

int handleResponseOfMailFrom(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != RECEIVE_MAIL_FROM_RESPONSE || connection->prev_state != SEND_MAIL_FROM)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    int size;
    char message[BUFFER];
    size = recv(connection->id, message, BUFFER, NULL);
    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfMailFrom: Fail to receive MAIL FROM response from server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);
    if (status != 250)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfMailFrom: Fail status(%d) of MAIL FROM response from server %s.", status, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    char verifyMessage[9] = "250 Ok\r\n\0";
    status = strncmp(message, verifyMessage, 8);
    if (status != 0)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfMailFrom: Fail status(%d) of MAIL FROM response from server %s.", status, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = SEND_RCPT_TO;

    return 0;
}

int handleSendRCPTto(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != SEND_RCPT_TO || connection->prev_state != RECEIVE_MAIL_FROM_RESPONSE)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    char message[BUFFER];
    int len;

    memset(message, '\0', BUFFER);
    sprintf(message, "RCPT To: <%s>\r\n", connection->meta_data.to);
    len = strlen(message);

    int size = send(connection->id, message, len, NULL);
    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleSendRCPTto: Fail to send RCPT on server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_RCPT_TO_RESPONSE;

    return 0;
}

int handleResponseOfRCPTto(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != RECEIVE_RCPT_TO_RESPONSE || connection->prev_state != SEND_RCPT_TO)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    int size;
    char message[BUFFER];
    size = recv(connection->id, message, BUFFER, NULL);
    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfRCPTto: Fail to receive RCPT TO response from server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);
    if (status != 250)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfRCPTto: Fail status(%d) of RCPT TO response from server %s.", status, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    char verifyMessage[9] = "250 Ok\r\n\0";
    int len = strlen(message);
    status = strncmp(message, verifyMessage, 8);
    if (status != 0)
    {
        char err_message[150 + len];
        memset(err_message, '\0', 150 + len);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfMailFrom: Fail message(%s) of RCPT TO response from server %s.", message, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = SEND_DATA;

    return 0;
}

int handleSendDATA(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != SEND_DATA || connection->prev_state != RECEIVE_RCPT_TO_RESPONSE)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    char message[7];

    memset(message, '\0', 7);
    sprintf(message, "DATA\r\n");

    int size = send(connection->id, message, 7, NULL);
    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleSendDATA: Fail to send DATA on server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_DATA_RESPONSE;

    return 0;   
}

int handleResponseOfDATA(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != RECEIVE_DATA_RESPONSE || connection->prev_state != SEND_DATA)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    int size;
    char message[BUFFER];
    size = recv(connection->id, message, BUFFER, NULL);
    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfDATA: Fail to receive DATA response from server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);
    if (status != 354)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfDATA: Fail status(%d) of DATA response from server %s.", status, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = SEND_LETTER;

    return 0;
}

int handleSendLetter(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != SEND_LETTER || connection->prev_state != RECEIVE_DATA_RESPONSE)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }   

    if (!connection->meta_data.file)
    {
        connection->meta_data.file = fopen(connection->task_pool->path, "r");

        if (!connection->meta_data.file)
        {
            char err_message[150];
            memset(err_message, '\0', 150);
            sprintf(err_message, "Worker: SMTP_Control: handleSendLetter: Fail to open file (%s). Errno: %d", connection->task_pool->path, errno);
            Error(err_message);

            //  change state on Error
            connection->current_state = SMTP_ERROR;
            return -2;
        }
    }

    //  read bytes and sending
    bool state = true;
    int max_buffer_on_smtp = 1000;
    char *pointer;
    int size;
    int len;
    while(state)
    {
        memset(connection->meta_data.message,'\0', max_buffer_on_smtp);
        pointer = fgets(connection->meta_data.message, max_buffer_on_smtp-2, connection->meta_data.file);
        if (pointer == NULL)
        {
            //  need send <CRLF>.<CRLF>
            strcpy(connection->meta_data.message, "\r\n.\r\n");
            state = false;
        }

        len = strlen(connection->meta_data.message);

        size = send(connection->id, connection->meta_data.message, len, NULL);

        if (size < 0)
        {
            if (size == -1 && (errno == EWOULDBLOCK || errno == EINPROGRESS)) 
            {
                //  All ok, connection is would block, wait and again try to receive getting
                return 1;
            }

            char err_message[150];
            memset(err_message, '\0', 150);
            sprintf(err_message, "Worker: SMTP_Control: handleSendLetter: Fail to send Letter to server %s. Errno: %d", connection->mx_record, errno);
            Error(err_message);

            //  change state on Error
            connection->current_state = SMTP_ERROR;
            return -2;
        }
    }

    fclose(connection->meta_data.file);
    connection->meta_data.file = NULL;

    //  change states
    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_LETTER_RESPONSE;

    return 0;
}

int handleResponseOfLetter(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != RECEIVE_LETTER_RESPONSE || connection->prev_state != SEND_LETTER)
    {
        //  Set error state
        connection->current_state = SMTP_ERROR;
        return -1;
    }

    int size;
    char message[BUFFER];
    size = recv(connection->id, message, BUFFER, NULL);
    if (size < 0)
    {
        if (size == -1 && errno == EWOULDBLOCK)
        {
            //  All ok, connection is would block, wait and again try to receive getting
            return 1;
        }

        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfLetter: Fail to receive response about letter from server %s. Errno: %d", connection->mx_record, errno);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);
    if (status != 250)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfLetter: Fail status(%d) of response about letter from server %s.", status, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    char verifyMessage[9] = "250 Ok\r\n\0";
    int len = strlen(message);
    status = strncmp(message, verifyMessage, 8);
    if (status != 0)
    {
        char err_message[150 + len];
        memset(err_message, '\0', 150 + len);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfMailFrom: Fail message(%s) of RCPT TO response from server %s.", message, connection->mx_record);
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    //  current task is resolved, we needed to move letter in current and destroy task
    int len = strlen(connection->task_pool->path);
    len += 3;
    char *new_filepath = (char *)calloc(len, sizeof(char));

    if (!new_filepath)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfLetter: Fail to allocate memory for new letter path");
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -5;
    }

    status = SetPathInCurrentDirectory(new_filepath, connection->task_pool->path);
    if (status < 0)
    {
        char err_message[150];
        memset(err_message, '\0', 150);
        sprintf(err_message, "Worker: SMTP_Control: handleResponseOfLetter: Fail to set move path for letter");
        Error(err_message);

        //  change state on Error
        connection->current_state = SMTP_ERROR;
        return -6;
    }

    MoveLetter(connection->task_pool->path, new_filepath);

    int next_state;

    if (connection->task_pool->next)
    {
        next_state = SEND_MAIL_FROM;
        struct worker_task *pointer = connection->task_pool->next;
        DestroyTask(connection->task_pool);
        connection->task_pool = pointer;
    }
    else 
    {
        next_state = SEND_QUIT;
        DestroyTask(connection->task_pool);
        connection->task_pool = NULL;
    }

    connection->prev_state = connection->current_state;
    connection->current_state = next_state;

    return 0;
}

int handleSendQUIT(struct FileDesc *connection)
{
    
}

int handleResponseOfQUIT(struct FileDesc *connection)
{
}

int handleDisponsing(struct FileDesc *connection)
{
}

int handleSmtpError(struct FileDesc *connection)
{
}

int handleSolvableMistake(struct FileDesc *connection)
{
}

int handleResponseOfRSET(struct FileDesc *connection)
{
}

//============//
//    TOOLS   //
//============//

int getMXrecord(struct FileDesc *connection)
{
    int state = 0;
    int size;
    int len;
    __u_char answer[512];
    memset(answer, '\0', 512);

    len = strlen(MY_DOMAIN);
    if (strncmp(connection->domain, MY_DOMAIN, len) != 0)
    {
        size = res_query(connection->domain, C_IN, T_MX, answer, 512);
    
        if (size < 0)
        {
            //  current domain not resolved
            char message[150];
            memset(message, '\0', 150);
            sprintf(message, "Worker: handlePrepareSocketConnection: getMXrecord: Not found MX_RECORD for %s domain", connection->domain);
            Error(message);
            return -1;
        }

        ns_msg message;
        ns_rr rr;

        ns_initparse(answer, size, &message);
        char *pointer;
        
        size = ns_msg_count(message, ns_c_in);
        for (int J = 0; J < size; J++) 
        {
            ns_parserr(&message, ns_c_in, J, &rr);
            ns_sprintrr(&message, &rr, NULL, NULL, answer, sizeof(answer));
            pointer = strstr(answer, ".\t");
            len = strlen(answer) - strlen(pointer);
            answer[len] = '\0';
            break;
        }

        struct hostent *he;
        struct in_addr **addr_list;

        he = gethostbyname(answer);
        if (!he) 
        {
            char message[100];
            memset(message, '\0', 100);
            sprintf(message, "Worker: handlePrepareSocketConnection: Fail to inet_pton for %s domain", connection->domain);
            Error(message);
            return -3;
        }
        addr_list = (struct in_addr **)he->h_addr_list;

        for (int I = 0; addr_list[I] != NULL; I++)
        {
            strcpy(answer, inet_ntoa(*addr_list[I]));
            break;
        }
    }
    else
    {
        //  Заглушка
        strncpy(answer, "127.0.0.1", 16);
    }

    connection->mx_record = (char *)calloc(size + 1, sizeof(char));
    if (!connection->mx_record)
    {
        char message[150];
        memset(message, '\0', 150);
        sprintf(message, "Worker: handlePrepareSocketConnection: getMXrecord: Fail to allocate memory for %s domain mx-record", connection->domain);
        Error(message);
        return -2;
    }

    size = strlen(answer);
    memset(connection->mx_record, '\0', size + 1);
    strncpy(connection->mx_record, answer, size);
    return 0;
}

int getCommandStatus(char *message)
{
    int status = -1;
    char code[4];
    strncpy(code, message, 3);
    code[4] = '\0';

    status = atoi(code);
    return status;
}

int verifyServerDomain(char *message, char *domain)
{
    int state = 0;
    int len = strlen(domain);

    //  hack
    state = strncmp(domain, message + 4, len);

    if (state == 0)
    {
        if (message[4 + len + 1] != ' ')
        {
            state = -1;
        }
    }
    else
    {
        state = -1;
    }

    return state;
}

int setLetterMetaData(struct letter_info *info, char *filepath)
{
    FILE *letter;
    letter = fopen(filepath, "r");
    if (!letter)
    {
        char message[150];
        memset(message, '\0', 150);
        sprintf(message, "Worker: handleSendMailFrom: getLetterMetaData: fail to find letter by path %s for domain", filepath);
        Error(message);
        return -1;
    }

    int state = 0;
    char buffer[BUFFER];
    char *pointer;
    int len;
    free(info->from);
    free(info->to);
    info->from = NULL;
    info->to = NULL;
    while (state == 0)
    {
        if (fgets(buffer, BUFFER, letter) == NULL)
        {
            state = -2;
            break;
        }

        if (info->to && info->from)
        {
            //  All ok, find all part of metadata
            break;
        }

        len = strlen(buffer);

        if (buffer[len - 2] != '\r' || buffer[len - 1] != '\n')
        {
            continue;
        }

        if (buffer[0] == 'T' && buffer[1] == 'o' && buffer[2] == ':')
        {
            //  find strict "To:"
            pointer = strstr(buffer, "@");
            if (!pointer)
            {
                continue;
            }

            len = strlen(pointer);
            info->to = (char *)calloc(len - 3, sizeof(char));
            if (!info->to)
            {
                Error("Worker: handleSendMailFrom: getLetterMetaData: fail to allocate memory for 'to' directive");
                return -3;
            }
            strncpy(info->to, pointer + 1, len - 3);
            continue;
        }

        if (buffer[0] == 'F' && buffer[1] == 'r' && buffer[2] == 'o' && buffer[3] == 'm' && buffer[4] == ':')
        {
            //  find strict "From:"
            pointer = strstr(buffer, "@");
            if (!pointer)
            {
                continue;
            }

            len = strlen(pointer);
            info->from = (char *)calloc(len - 3, sizeof(char));
            if (!info->from)
            {
                Error("Worker: handleSendMailFrom: getLetterMetaData: fail to allocate memory for 'from' directive");
                return -4;
            }
            strncpy(info->from, pointer + 1, len - 3);
            continue;
        }
    }

    fclose(letter);
    return state;
}