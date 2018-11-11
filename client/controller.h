#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include "structs.h"
#include "smtp_socket.h"
#include "file_viewer.h"

#define STATE_START_INIT 0
#define STATE_FINISH_INIT 1
#define STATE_START_WORK 2
#define STATE_FINISH_WORK 3
#define STATE_FAIL_WORK -1

struct Controller
{
    struct FileDescSet readers;
    struct FileDescSet writers;
    struct FileDescSet handlers;
    int currentState;
    bool worked;
};

void InitController();
void Run();
void Stop();
void Dispose();

#endif // CONTROLLER_H