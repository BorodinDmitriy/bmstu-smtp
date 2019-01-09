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

int prepareSocketForConnection(struct FileDesc *connection);
int handleGreeting(struct FileDesc *connection);
int handleSendEHLO(struct FileDesc *connection);
int handleResponseOfEHLO(struct FileDesc *connection);
int handleSendMailFrom(struct FileDesc *connection);
int handleResponseOfMailFrom(struct FileDesc *connection);

//==========================//
//      PUBLIC METHODS      //
//==========================//

// int CloseConnection(struct FileDesc fd)
// {
//     int state = 0;
//     state = shutdown(fd.id, 2);
//     if (state)
//     {
//         printf("Fail to shutdown connection\r\n");
//         return state;
//     }

//     state = close(fd.id);
//     if (state)
//     {
//         printf("Fail to close connection\r\n");
//         return state;
//     }

//     return 0;
// }

//  Send letter
// int SendMail(int fd, struct Mail letter)
// {
//     fd = socketList.sockets[0].fd;
//     int cur = findIndex(fd);

//     if (cur == -1)
//     {
//         printf("Not found sender socket");
//         return -1;
//     }

//     memset(&socketList.sockets[cur].dest, '0', sizeof(socketList.sockets[cur].dest));

//     socketList.sockets[cur].dest.sin_family = AF_INET;
//     socketList.sockets[cur].dest.sin_port = htons(SMTP_PORT);

//     int state = 0;

//     // state = inet_pton(AF_INET, "94.100.180.160", &socketList.sockets[cur].dest.sin_addr);
//     state = inet_pton(AF_INET, "127.0.0.1", &socketList.sockets[cur].dest.sin_addr);
//     if (state <= 0)
//     {
//         printf("\nFail to convert address");
//         return -1;
//     }

//     state = connect(fd, (struct sockaddr *)&socketList.sockets[cur].dest, sizeof(socketList.sockets[cur].dest));
//     if (state < 0)
//     {
//         printf("\nConnection failed\n");
//         return -1;
//     }

//     sendHelo(cur, "IU7.2@yandex.ru");

//     //  close connection
//     shutdown(fd, 2);
//     close(fd);
//     return 0;
// }

//==========================//
//      PRIVATE METHODS     //
//==========================//

// int sendCommand(struct FileDesc *fd, char *command, char *data)
// {
//     int state = 0;

//     int send_len = (int)strlen(command) + 1 + (int)strlen(data) + 5;

//     char *message = (char *)calloc(send_len, sizeof(char));
//     strcpy(message, command);
//     strcat(message, " ");
//     strcat(message, data);
//     strcat(message, " \r\n");

//     state = send(fd->id, message, strlen(message), 0);
//     free(message);
//     return state;
// }

// //  send "HELO" message of SMTP
// int sendHelo(int index, char *address)
// {
//     char *message = (char *)calloc(100, sizeof(char));
//     int state;

//     // state = recv(socketList.sockets[index].fd, message, 100, NULL);
//     if (state <= 0)
//     {
//         printf("\nFail to receive 'HELO' from server");
//         free(message);
//     }

//     free(message);
//     int address_len = strlen(address);
//     int message_len = 5 + address_len + 5;
//     *message = (char *)calloc(address_len, sizeof(char));

//     memset(message, ' ', address_len);
//     strcpy(message, "HELO ");
//     strcat(message, address);
//     strcat(message, " \r\n");

//     // state = send(socketList.sockets[index].fd, message, message_len, 0);

//     if (state < 0)
//     {
//         printf("\nFail to send 'HELO' to %s", address);
//         free(message);
//         return state;
//     }
//     memset(message, '\0', address_len);
//     printf("\nSend HELO from %d socket", index);
//     // int readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

//     printf("\nReceive response from HELO: %s", message);

//     memset(message, ' ', address_len);
//     strcpy(message, "QUIT ");
//     strcat(message, "\r\n");

//     // state = send(socketList.sockets[index].fd, message, message_len, 0);

//     if (state < 0)
//     {
//         printf("\nFail to send 'QUIT' to %s", address);
//         free(message);
//         return state;
//     }

//     memset(message, '\0', address_len);
//     printf("\nSend HELO from %d socket", index);
//     // readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

//     printf("\nReceive response from HELO: %s", message);
//     free(message);
//     return 0;
// }

int SMTP_Control(struct FileDesc *socket_connection)
{
    int state = 0;
    while (state == 0)
    {
        switch (socket_connection->current_state)
        {
        case PREPARE_SOCKET_CONNECTION:
            state = prepareSocketForConnection(socket_connection);
            break;

        case CONNECT:
            state = handleGreeting(socket_connection);
            break;

        case RECEIVE_SMTP_GREETING:
            state = handleSendEHLO(socket_connection);
            break;

        case SEND_EHLO:
            state = handleResponseOfEHLO(socket_connection);
            break;

        case RECEIVE_EHLO_RESPONSE:
            state = handleSendMailFrom(socket_connection);
            break;

        case SEND_MAIL_FROM:
            state = handleResponseOfMailFrom(socket_connection);
            break;

        default:
            state = -1;
        }
    }
    return state;
}

//============//
//  HANDLERS  //
//============//

int prepareSocketForConnection(struct FileDesc *connection)
{
    int state = 0;

    //  Check states
    if (connection->current_state != PREPARE_SOCKET_CONNECTION || connection->prev_state != NULL_POINTER)
    {
        char message[150];
        memset(message, '\0', 150);
        sprintf(message, "Worker: prepareSocketForConnection: unexpected current_state (%d) and prev_state(%d) for socket with domain %s", connection->current_state, connection->prev_state, connection->domain);
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
        sprintf(message, "Worker: prepareSocketForConnection: Fail to set socket option 'SO_REUSEADDR' for %s domain", connection->domain);
        Error(message);
        return -3;
    }

    state = setsockopt(connection->id, SOL_SOCKET, SO_REUSEPORT, (char *)&opt_val, opt_len);
    if (state)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: prepareSocketForConnection: Fail to set socket option 'SO_REUSEPORT' for %s domain", connection->domain);
        Error(message);
        return -4;
    }

    int file_flags = fcntl(connection->id, F_GETFL, 0);
    if (file_flags == -1)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: prepareSocketForConnection: Fail to receive socket flags for %s domain", connection->domain);
        Error(message);
        return -5;
    }

    state = fcntl(connection->id, F_SETFL, file_flags | O_NONBLOCK);
    if (state)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: prepareSocketForConnection: Fail to set flag 'O_NONBLOCK' for socket by %s domain", connection->domain);
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
        sprintf(message, "Worker: prepareSocketForConnection: Fail to inet_pton for %s domain", connection->domain);
        Error(message);
        return -7;
    }

    state = connect(connection->id, (struct sockaddr *)&connection->addr, sizeof(connection->addr));
    if (state < 0 && state != -1 && errno != EAGAIN)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker: prepareSocketForConnection: Fail to inet_pton for %s domain", connection->domain);
        Error(message);
        return -8;
    }

    //  CHANGE CURRENT STATE AND PREV STATE
    connection->current_state = CONNECT;
    connection->prev_state = PREPARE_SOCKET_CONNECTION;

    if (state == -1 && errno == EAGAIN)
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
    if (connection->current_state != CONNECT || connection->prev_state != PREPARE_SOCKET_CONNECTION)
    {
        //  Set error state
        connection->prev_state = connection = connection->current_state;
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
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);

    //  Unexpected status codes
    if (status != 220 && status != 421)
    {
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    if (status == 421)
    {
        connection->prev_state = connection->current_state;
        connection->current_state = DISPOSING_SOCKET;
        return 0;
    }

    status = verifyServerDomain(message, connection->domain);

    if (status != 0)
    {
        //  Unexpected domain
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    //  Successfully change state, all actions is ok
    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_SMTP_GREETING;

    return 0;
}

int handleSendEHLO(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != RECEIVE_SMTP_GREETING || connection->prev_state != CONNECT)
    {
        //  Set error state
        connection->prev_state = connection = connection->current_state;
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
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    //  Change current state
    connection->prev_state = connection->current_state;
    connection->current_state = SEND_EHLO;

    return 0;
}

int handleResponseOfEHLO(struct FileDesc *connection)
{
    //  Check current and prev states
    if (connection->current_state != SEND_EHLO || connection->prev_state != RECEIVE_SMTP_GREETING)
    {
        //  Set error state
        connection->prev_state = connection = connection->current_state;
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
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -2;
    }

    int status = getCommandStatus(message);
    if (status != 250 && status != 421)
    {
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -3;
    }

    if (status == 421)
    {
        connection->prev_state = connection->current_state;
        connection->current_state = DISPOSING_SOCKET;
        return 0;
    }

    //  check message
    int len = strlen(MY_DOMAIN);
    char *pointer = strstr(message, MY_DOMAIN);
    if (!pointer)
    {
        //  In EHLO not founded my name... is error connection
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -4;
    }

    //  check CRLF
    if (message[size - 2] != '\r' || message[size - 1] != '\n' || (pointer + len + 1) != '\r' || (pointer - 1) != ' ')
    {
        //  Bad command. Error in CRLF or my domain
        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;
        return -5;
    }

    connection->attempt = 0;

    //  change current state
    connection->prev_state = connection->current_state;
    connection->current_state = RECEIVE_EHLO_RESPONSE;

    return 0;
}

int handleSendMailFrom(struct FileDesc *connection)
{
    //  Check states and transition

    bool first_sending = ((connection->current_state == RECEIVE_EHLO_RESPONSE) && (connection->prev_state == SEND_EHLO));
    bool repeated_sending = ((connection->current_state == RECEIVE_LETTER_RESPONSE) && (connection->prev_state == SEND_LETTER));

    if (!first_sending || !repeated_sending)
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
        sprintf(message, "Worker: handleSendMailFrom: Fail to receive metadata about letter with filepath(%s) by domain (%s)",connection->task_pool->path, connection->domain);
        Error(message);

        connection->prev_state = connection->current_state;
        connection->current_state = SMTP_ERROR;

        return -2;
    }

    //  send MAIL command

    char message[BUFFER];
    memset(message, '\0', BUFFER);
    sprintf(message, "MAIL FROM:<%s>\r\n",connection->meta_data.from);
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
    connection->current_state = SEND_MAIL_FROM;

    return 0;
}

//============//
//    TOOLS   //
//============//

int getMXrecord(struct FileDesc *connection)
{
    int state = 0;
    int size;
    __u_char answer[512];
    memset(answer, '\0', 512);

    if (strcmp(connection->domain, "smtp-test.ru") != 0)
    {
        size = res_search(connection->domain, C_IN, T_MX, answer, 512);
    }
    else
    {
        //  Заглушка
        strncpy(answer, "127.0.0.1", 16);
        size = 15;
    }
    if (size < 0)
    {
        //  current domain not resolved
        char message[150];
        memset(message, '\0', 150);
        sprintf(message, "Worker: prepareSocketForConnection: getMXrecord: Not found MX_RECORD for %s domain", connection->domain);
        Error(message);
        return -1;
    }

    connection->mx_record = (char *)calloc(size + 1, sizeof(char));
    if (!connection->mx_record)
    {
        char message[150];
        memset(message, '\0', 150);
        sprintf(message, "Worker: prepareSocketForConnection: getMXrecord: Fail to allocate memory for %s domain mx-record", connection->domain);
        Error(message);
        return -2;
    }

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

    status = itoa(code);
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