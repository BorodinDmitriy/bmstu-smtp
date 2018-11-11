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

int GiveControlToSocket(struct FileDesc *fd);
void CloseAll();

#define BUFFER 4096

#endif //SMTP_SOCKET_H