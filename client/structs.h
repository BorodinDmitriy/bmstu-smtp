#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include "states.h"
#include "./../common/header.h"

//======================//
//      FD TYPES        //
//======================//
#define SOCKET_FD 0
#define FILE_FD 1


struct FileDescSet
{
    fd_set set;
    struct FileDescList *list;
    int count;
};

struct FileDesc
{
    int id;
    int type;
    struct sockaddr_in addr;
    int context;
    int goal;
};

struct FileDescList
{
    struct FileDesc fd;
    struct FileDescList *next;
};

struct domain_record
{
    char* domain;
    int workerId;
};



#endif //  STRUCTS_H