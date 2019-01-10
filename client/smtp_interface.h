#ifndef SMTP_SOCKET_H
#define SMTP_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "structs.h"

int SMTP_Control(struct FileDesc *socket_connection);

#endif //SMTP_SOCKET_H