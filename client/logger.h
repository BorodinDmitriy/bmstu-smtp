#ifndef LOGGER_H
#define LOGGER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mqueue.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "structs.h"

#define MAX_MSG_SIZE 1024
#define MAX_COUNT_MSG 2
#define QUEUE_NAME "/test_queue"
#define LOG_FILE "app.log"


int InitLogger();
void Error(char *message);

#endif //LOGGER_H