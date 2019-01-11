#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "utils.h"
#include "test_smtp.h"

// sudo apt-get install libcunit1 libcunit1-doc libcunit1-dev

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
    {
        printf("%s\n", CU_get_error_msg());
        exit(CU_get_error());
    }

    CU_pSuite suite = CUnitCreateSuite("suite_test");
    if (suite)
    {
        add_to_suit_smtp_test(suite);
    }
    else
    {
        CU_cleanup_registry();
        printf("%s\n", CU_get_error_msg());
        exit(CU_get_error());
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    printf("\n");
    CU_basic_show_failures(CU_get_failure_list());
    printf("\n");
    return CU_get_error();
}
