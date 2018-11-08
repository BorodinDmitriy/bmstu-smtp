#ifndef SMTP_SOCKET_H
#define SMTP_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "structs.h"

struct smtpSocket
{
    int fd;
    struct sockaddr_in dest;
};

struct ListSocket 
{
    int count;
    struct smtpSocket * sockets;
};

void InitSmtpSockets(struct FileDescSet * fdSet);
int SendMail(int fd, struct Mail letter);
void DisposeSmtpSockets();

#define BUFFER 4096
#define SOCK_COUNT 4

#endif //SMTP_SOCKET_H