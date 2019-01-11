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

TEST_FUNCT(create_client_socket_has_message) {
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;
	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(sizeof(c_sock->buffer),8);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_INIT);
	CU_ASSERT_PTR_NOT_EQUAL(c_sock->message,NULL);
	CU_ASSERT_EQUAL(c_sock->message->recepients_num,0);
	free(c_sock);
}

TEST_FUNCT(handle_helo_state_init) {
	char *msg_buffer = "HELO myserver.com";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_HELO(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

int add_to_suit_smtp_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, create_client_socket_no_message)
        ADD_SUITE_TEST(suite, create_client_socket_has_message)
        ADD_SUITE_TEST(suite, handle_helo_state_init)
    }
    return CU_get_error();
}