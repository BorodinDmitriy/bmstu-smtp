#include "smtp_socket.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//
int sendCommand(struct FileDesc *fd, char *command, char *data);
int recvCommand(struct FileDesc *fd);

//==========================//
//      PUBLIC METHODS      //
//==========================//

int GiveControlToSocket(struct FileDesc *fd)
{
    int state = 0;
    switch ((*fd).context)
    {
    case START_WORK:
    {
        // state = smtpConnection(fd);
    }
    case RECEIVE_HELO_MESSAGE:
    {
        state = smtpHELO(fd, 0);
        break;
    }
    case SEND_HELO_MESSAGE:
    {
        state = smtpHELO(fd, 1);
        break;
    }
    case RECEIVE_HELO_CONNECT:
    {
        state = smtpHELO(fd, 2);
        break;
    }
    default:
        return -1;
    }

    return state;
}

int SmtpInitSocket(char *domain, struct FileDesc *fd)
{
    int state = 0;

    struct FileDesc connection;

    connection.id = socket(AF_INET, SOCK_STREAM, 0);
    connection.type = SOCKET_FD;

    int opt_val = 1;
    socklen_t opt_len = sizeof(opt_val);

    state = setsockopt(connection.id, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, opt_len);
    if (state)
    {
        printf("Fail to set socket option 'SO_REUSEADDR'\r\n");
        return state;
    }

    state = setsockopt(connection.id, SOL_SOCKET, SO_REUSEPORT, (char *)&opt_val, opt_len);
    if (state)
    {
        printf("Fail to set socket option 'SO_REUSEPORT'\r\n");
        return state;
    }

    int file_flags = fcntl(connection.id, F_GETFL, 0);
    if (file_flags == -1)
    {
        printf("Fail to receive socket flags");
        return file_flags;
    }

    state = fcntl(connection.id, F_SETFL, file_flags | O_NONBLOCK);
    if (state)
    {
        printf("Fail to set flag 'O_NONBLOCK' for socket");
        return state;
    }

    connection.addr.sin_family = AF_INET;
    connection.addr.sin_port = htons(SERVER_PORT);

    state = inet_pton(AF_INET, domain, &connection.addr.sin_addr);
    if (state <= 0)
    {
        printf("Fail to inet_pton");
        return state;
    }

    state = connect(connection.id, (struct sockaddr *)&connection.addr, sizeof(connection.addr));
    if (state < 0) {
        printf("Fail connection");
        return state;
    }

    connection.context = START_WORK;

    *fd = connection;

    return 0;
}

int smtpConnection(struct FileDesc fd) 
{
 //   int state = connect(fd.id, &fd.addr, sizeof(.dest));
}

int CloseConnection(struct FileDesc fd)
{
    int state = 0;
    state = shutdown(fd.id, 2);
    if (state)
    {
        printf("Fail to shutdown connection\r\n");
        return state;
    }

    state = close(fd.id);
    if (state)
    {
        printf("Fail to close connection\r\n");
        return state;
    }

    return 0;
}

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

int sendCommand(struct FileDesc *fd, char *command, char *data)
{
    int state = 0;

    int send_len = (int)strlen(command) + 1 + (int)strlen(data) + 5;

    char *message = (char *)calloc(send_len, sizeof(char));
    strcpy(message, command);
    strcat(message, " ");
    strcat(message, data);
    strcat(message, " \r\n");

    state = send(fd->id, message, strlen(message), 0);
    free(message);
    return state;
}

int recvCommand(struct FileDesc *fd)
{
    return 0;
}

int smtpHELO(struct FileDesc *fd, int process_state)
{
    int state = 0;
    if (process_state <= 0 && state >= 0)
    {
        state = recvCommand(fd);
        if (state == EWOULDBLOCK)
        {
            return state;
        }
    }

    if (process_state <= 1 && state >= 0)
    {
        state = sendCommand(fd, "HELO", "");
        if (state == EWOULDBLOCK)
        {
            return state;
        }
    }

    if (process_state <= 2 && state >= 0)
    {
        state = recvCommand(fd);
        if (state == EWOULDBLOCK)
        {
            return state;
        }
    }

    return state;
}

//  send "HELO" message of SMTP
int sendHelo(int index, char *address)
{
    char *message = (char *)calloc(100, sizeof(char));
    int state;

    // state = recv(socketList.sockets[index].fd, message, 100, NULL);
    if (state <= 0)
    {
        printf("\nFail to receive 'HELO' from server");
        free(message);
    }

    free(message);
    int address_len = strlen(address);
    int message_len = 5 + address_len + 5;
    *message = (char *)calloc(address_len, sizeof(char));

    memset(message, ' ', address_len);
    strcpy(message, "HELO ");
    strcat(message, address);
    strcat(message, " \r\n");

    // state = send(socketList.sockets[index].fd, message, message_len, 0);

    if (state < 0)
    {
        printf("\nFail to send 'HELO' to %s", address);
        free(message);
        return state;
    }
    memset(message, '\0', address_len);
    printf("\nSend HELO from %d socket", index);
    // int readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

    printf("\nReceive response from HELO: %s", message);

    memset(message, ' ', address_len);
    strcpy(message, "QUIT ");
    strcat(message, "\r\n");

    // state = send(socketList.sockets[index].fd, message, message_len, 0);

    if (state < 0)
    {
        printf("\nFail to send 'QUIT' to %s", address);
        free(message);
        return state;
    }

    memset(message, '\0', address_len);
    printf("\nSend HELO from %d socket", index);
    // readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

    printf("\nReceive response from HELO: %s", message);
    free(message);
    return 0;
}
