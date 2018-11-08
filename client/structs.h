#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include "./../common/header.h"

#define SMTP_PORT 2525

struct FileDescSet
{
    fd_set set;
    int count;
};

#endif //  STRUCTS_H