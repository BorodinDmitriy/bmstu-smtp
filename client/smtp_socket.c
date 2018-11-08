#include "smtp_socket.h"

struct smtpSocket clientSocket;
struct ListSocket socketList;

void InitSmtpSocket(struct FileDescSet * fdSet)
{
    socketList.sockets = (struct Socket *)calloc(SOCK_COUNT, sizeof(struct smtpSocket));
    if (socketList.sockets == NULL)
    {
        printf("Failure of memory allocations");
        exit(-2);
    }

    for (int i = 0; i < SOCK_COUNT; i++)
    {
        socketList.sockets[i].fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socketList.sockets[i].fd < 0 )
        {
            printf("Failure to create socket[%d]", i);
            exit(-2);
        }
        fd_set r;

        FD_SET(socketList.sockets[i].fd, &(*fdSet).set);
        socketList.count++;
    }

    return;
}

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

    state = inet_pton(AF_INET, "94.100.180.160", &socketList.sockets[cur].dest.sin_addr);
    if (state <= 0) 
    {
        printf("\nFail to convert address");
        return -1;
    }

    state = connect(fd, (struct sockaddr *) & socketList.sockets[cur].dest, sizeof(socketList.sockets[cur].dest));
    if (state < 0) 
    {
        printf("\nConnection failed\n");
        return -1;
    }

    sendHelo(cur, "127.0.0.1");

    return 0;
}

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

int sendHelo(int index, char * address) 
{
    int address_len = strlen(address);
    int message_len = 5 + address_len + 5;
    char * message = (char *) calloc(address_len, sizeof(char));

    strcpy(message, "HELO ");
    strcat(message, address);
    strcat(message, " CRLF");

    int state;
    state = send(socketList.sockets[index].fd, message, message_len, 0);

    if (state < 0) 
    {
        printf("\nFail to send 'HELO' to %s", address);
        return state;
    }
    printf("\nSend HELO from %d socket", index);
    int readed = recv(socketList.sockets[index].fd, message, message_len, NULL);

    printf("\nReceive response from HELO: %s", message);
    free(message);
    return 0;
}