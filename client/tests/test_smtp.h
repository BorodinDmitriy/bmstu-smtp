#ifndef TEST_SMTP_H
#define TEST_SMTP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <CUnit/Basic.h>
#include "utils.h"
#include "../smtp_interface.h"
#include "../states.h"
#include "../structs.h"

int add_to_suit_smtp_test(CU_pSuite suite);

#endif