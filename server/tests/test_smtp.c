#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "utils.h"

#include "../smtp.h"
#include "../socket.h"

TEST_FUNCT(create_client_socket_no_message) {
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;
	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 0;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(sizeof(c_sock->buffer),8);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_INIT);
	CU_ASSERT_PTR_EQUAL(c_sock->message,NULL);
	free(c_sock);
}

int add_to_suit_smtp_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, create_client_socket_no_message)
    }
    printf("BLA\n");
    return CU_get_error();
}