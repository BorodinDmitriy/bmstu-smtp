#include "smtp_socket.h"

struct smtpSocket clientSocket;
struct smtpSocket *Sockets;

void InitSmtpSocket(int *fdSet, int *pointer, int limit)
{
    int countSocket = SOCK_COUNT;
    if (limit - SOCK_COUNT < 0)
    {
        printf("Lack of memory for create any sockets");
        exit(-2);
    }

    Sockets = (struct Socket *)calloc(countSocket, sizeof(struct smtpSocket));
    if (Sockets == NULL)
    {
        printf("Failure of memory allocations");
        exit(-2);
    }

    for (int i = 0; i < SOCK_COUNT; i++)
    {
        Sockets[i].fd = socket(AF_INET, SOCK_STREAM, 0);
        fdSet[*pointer] = Sockets[i].fd;
        if (Sockets[i].fd < 0 || pointer + 1 > limit)
        {
            printf("Failure to create socket[%d]", i);
            exit(-2);
        }
        *pointer++;
    }

    return;
}

int Send()
{
    return 0;
}