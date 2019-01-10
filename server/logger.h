#ifndef LOGGER_H
#define LOGGER_H

#include "../common/header.h"
#include "process.h"

int init_logger( struct sockaddr_in serv_address);
int run_logger(struct process *pr);
int dispose_logger();

#endif