#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include "structs.h"
#include "smtp_socket.h"

#define INIT_FD_SET_COUNT 10

struct Controller
{
    struct FileDescSet readers;
    struct FileDescSet writers;
    struct FileDescSet handlers;
};

void InitController();

#endif // CONTROLLER_H