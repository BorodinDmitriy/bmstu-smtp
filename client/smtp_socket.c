#include "smtp_socket.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

//==========================//
//      PUBLIC METHODS      //
//==========================//

int GiveControlToSocket(struct FileDesc *fd)
{
    int state = 0;
    switch ((*fd).context) 
    {
        case RECEIVE_HELO_MESSAGE:
            state = smtpHELO(fd, 0);
            break;
        
        case SEND_HELO_MESSAGE:
            state = smtpHELO(fd, 1);
            break;

        case RECEIVE_HELO_CONNECT:
            state = smtpHELO(fd, 2);
            break;
        
        default: 
            return -1;
    }

    return state;
}

//  Send letter
int SendMail(int fd, struct Mail letter)
{
    fd = socketList.sockets[0].fd;
    int cur = findIndex(fd);

    if (cur == -1)
    {
        printf("Not found sender socket");
        return -1;
    }

    memset(&socketList.sockets[cur].dest, '0', sizeof(socketList.sockets[cur].dest));

    socketList.sockets[cur].dest.sin_family = AF_INET;
    socketList.sockets[cur].dest.sin_port = htons(SMTP_PORT);

    int state = 0;

    // state = inet_pton(AF_INET, "94.100.180.160", &socketList.sockets[cur].dest.sin_addr);
    state = inet_pton(AF_INET, "127.0.0.1", &socketList.sockets[cur].dest.sin_addr);
    if (state <= 0)
    {
        printf("\nFail to convert address");
        return -1;
    }

    state = connect(fd, (struct sockaddr *)&socketList.sockets[cur].dest, sizeof(socketList.sockets[cur].dest));
    if (state < 0)
    {
        printf("\nConnection failed\n");
        return -1;
    }

    sendHelo(cur, "IU7.2@yandex.ru");

    //  close connection
    shutdown(fd, 2);
    close(fd);
    return 0;
}

//  Dispose resource
void DisposeSmtpSockets()
{
    printf("\n\nDispose SMTP sockets\n");
    for (int i = 0; i < socketList.count; i++)
    {
        close(socketList.sockets[i].fd);
        free(socketList.sockets);
        printf("Close socket %d\n", i);
    }

    printf("\nDispose SMTP sockets... success\n");
    return;
}

//==========================//
//      PRIVATE METHODS     //
//==========================//

//  find current socket
int findIndex(int fd)
{
    int find = -1;

    for (int I = 0; I < socketList.count; I++)
    {
        if (fd == socketList.sockets[I].fd)
        {
            find = I;
            break;
        }
    }

    return find;
}

//  send "HELO" message of SMTP
int sendHelo(int index, char *address)
{
    char *message = (char *)calloc(100, sizeof(char));
    int state;

    state = recv(socketList.sockets[index].fd, message, 100, NULL);
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

    state = send(socketList.sockets[index].fd, message, message_len, 0);

    if (state < 0)
    {
        printf("\nFail to send 'HELO' to %s", address);
        free(message);
        return state;
    }
    memset(message, '\0', address_len);
    printf("\nSend HELO from %d socket", index);
    int readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

    printf("\nReceive response from HELO: %s", message);

    memset(message, ' ', address_len);
    strcpy(message, "QUIT ");
    strcat(message, "\r\n");

    state = send(socketList.sockets[index].fd, message, message_len, 0);

    if (state < 0)
    {
        printf("\nFail to send 'QUIT' to %s", address);
        free(message);
        return state;
    }

    memset(message, '\0', address_len);
    printf("\nSend HELO from %d socket", index);
    readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

    printf("\nReceive response from HELO: %s", message);
    free(message);
    return 0;
}
