#ifndef TEST_SMTP_H
#define TEST_SMTP_H

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "utils.h"
#include "../smtp_interface.h"
#include "../states.h"
#include "../structs.h"

int add_to_suit_smtp_test(CU_pSuite suite);

#endif