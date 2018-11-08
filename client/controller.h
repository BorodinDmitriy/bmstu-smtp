#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include "smtp_socket.h"

#define INIT_FD_SET_COUNT 10

struct Controller
{
    int *fdSet;
    int pointer;
    int limit;
    int maxFD;
};

void InitController();

#endif // CONTROLLER_H