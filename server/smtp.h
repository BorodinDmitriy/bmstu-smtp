#ifndef SMTP_H
#define SMTP_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>

// #include <syslog.h>


#define STR_EQUAL(a,b)	(strcmp(a, b) == 0)

void smtp_handler(int *socket_fd, const int pid);

#endif