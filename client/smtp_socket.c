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

int Send()
{
    return 0;
}