#ifndef WORKER_H
#define WORKER_H

#include <stdbool.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include "structs.h"
#include "controller.h"

void InitWorker(void *my_info);

#endif //WORKER_H