#ifndef SMTP_H
#define SMTP_H

#include "../common/header.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>

// #include <syslog.h>




void smtp_handler(int *socket_fd, const int pid);

#endif