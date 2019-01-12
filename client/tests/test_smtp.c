#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "utils.h"
#include "test_smtp.h"

#include "../smtp_interface.h"

TEST_FUNCT(domain_is_NULL)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	int state = SMTP_Control(connection);

	CU_ASSERT_NOT_EQUAL(state, 0);
	CU_ASSERT_EQUAL(state, -2);
	free(connection);
}

TEST_FUNCT(bad_domain)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(10, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 10);
	strcpy(connection->domain, "bad-dom");

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	int state = SMTP_Control(connection);

	CU_ASSERT_NOT_EQUAL(state, 0);
	CU_ASSERT_EQUAL(state, -2);
	free(connection->domain);
	free(connection);
}

TEST_FUNCT(connection_is_blocked)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(10, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 10);
	strcpy(connection->domain, "mail.ru");

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	int state = SMTP_Control(connection);

	CU_ASSERT_NOT_EQUAL(state, 0);
	CU_ASSERT_EQUAL(state, 1);
	free(connection->domain);
	free(connection);
}

TEST_FUNCT(greeting_bad_graph_state)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(20, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 20);
	strcpy(connection->domain, TEST_SERVER_DOMAIN);

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	int state = SMTP_Control(connection);
	connection->prev_state = SEND_QUIT;

	state = SMTP_Control(connection);
	CU_ASSERT_EQUAL(state, -1);
	shutdown(connection->id, 0);
	close(connection->id);
	free(connection->domain);
	free(connection);
}

TEST_FUNCT(in_greeting_not_correct_domain)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(20, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 20);
	strcpy(connection->domain, TEST_SERVER_DOMAIN);

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	//  connect
	int state = SMTP_Control(connection);

	sleep(1);
	strncpy(connection->domain, "12345\0", 6);
	state = SMTP_Control(connection);
	CU_ASSERT_EQUAL(state, -4);

	shutdown(connection->id, 0);
	close(connection->id);
	free(connection->domain);
	free(connection);
}

TEST_FUNCT(in_greeting_421_status)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(20, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 20);
	strcpy(connection->domain, TEST_SERVER_DOMAIN);

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	//  connect
	int state = SMTP_Control(connection);

	sleep(60);
	state = SMTP_Control(connection);
	printf("%d\n", state);
	shutdown(connection->id, 0);
	close(connection->id);
	free(connection->domain);
	free(connection);
	CU_ASSERT_EQUAL(state, -3);
}

TEST_FUNCT(send_ehlo_and_correct_result)
{
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(20, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 20);
	strcpy(connection->domain, TEST_SERVER_DOMAIN);
	fd_set temp;
	FD_ZERO(&temp);

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	//  connect
	int state = SMTP_Control(connection);
	if (state == 1)
		sleep(1);

	//  receive GREETING
	state = SMTP_Control(connection);

	if (state == 1)
		sleep(1);

	//  send Ehlo
	state = SMTP_Control(connection);
	if (state == 1)
		sleep(2);

	//  response of Ehlo
	state = SMTP_Control(connection);

	//  blocked is ok
	if (state == 1)
	{
		state = 0;
	}

	shutdown(connection->id, 0);
	close(connection->id);
	free(connection->domain);
	free(connection);
	CU_ASSERT_EQUAL(state, 0);
}

TEST_FUNCT(send_ehlo_and_change_state_on_error)
{
	InitDictionary();
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(20, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 20);
	strcpy(connection->domain, TEST_SERVER_DOMAIN);
	connection->task_pool = NULL;
	fd_set temp;
	FD_ZERO(&temp);

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	//  connect
	int state = SMTP_Control(connection);
	if (state == 1)
	{
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, &temp, &temp, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	state = 0;
	while (state == 0)
	{
		//  receive GREETING
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, &temp, NULL, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	state = 1;
	//  send Ehlo
	while (state == 1)
	{
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, NULL, &temp, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	connection->current_state = SMTP_ERROR;
	//  resolve error
	state = SMTP_Control(connection);

	CU_ASSERT_EQUAL(connection->current_state, DISPOSING_SOCKET);
	//  Disposing
	state = SMTP_Control(connection);

	shutdown(connection->id, 0);
	close(connection->id);
	free(connection->domain);
	free(connection);
	FreeDictionary();
	CU_ASSERT_EQUAL(state, 0);
}

TEST_FUNCT(send_and_recv_ehlo_send_and_recv_mail_from_correct)
{
	InitDictionary();
	struct FileDesc *connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
	CU_ASSERT_PTR_NOT_EQUAL(connection, NULL);
	connection->domain = NULL;
	connection->domain = (char *)calloc(20, sizeof(char));
	CU_ASSERT_PTR_NOT_EQUAL(connection->domain, NULL);
	memset(connection->domain, '\0', 20);
	strcpy(connection->domain, TEST_SERVER_DOMAIN);
	connection->task_pool = NULL;
	connection->task_pool = (struct worker_task *)malloc(sizeof(struct worker_task));
	connection->task_pool->domain = NULL;
	connection->task_pool->next=NULL;
	connection->task_pool->path = NULL;
	connection->task_pool->domain = (char *)calloc(20, sizeof(char));
	memset(connection->task_pool->domain, '\0', 20);
	strcpy(connection->task_pool->domain, connection->domain);
	connection->task_pool->path = (char *)calloc(20, sizeof(char));
	memset(connection->task_pool->path, '\0', 20);
	strcpy(connection->task_pool->path, "fakemail.eml");
	fd_set temp;
	FD_ZERO(&temp);

	connection->prev_state = NULL_POINTER;
	connection->current_state = PREPARE_SOCKET_CONNECTION;

	//  connect
	int state = SMTP_Control(connection);
	if (state == 1)
	{
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, &temp, &temp, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	//  receive GREETING
	state = 1;
	while (state == 1)
	{	
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, &temp, NULL, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	
	//  send Ehlo
	state = 1;
	while (state == 1)
	{
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, NULL, &temp, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	//  receive Response
	state = 1;
	while (state == 1)
	{
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, &temp, NULL, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	CU_ASSERT_EQUAL(connection->current_state, SEND_MAIL_FROM);
	//  send MAIL FROM
	state = 1;
	while (state == 1)
	{
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, NULL, &temp, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}

	CU_ASSERT_EQUAL(connection->current_state, RECEIVE_MAIL_FROM_RESPONSE);
	//  receive MAIL FROM
	state = 1;
	while (state == 1)
	{
		state = SMTP_Control(connection);
		if (state == 0)
			break;
		FD_SET(connection->id, &temp);
		pselect(connection->id + 1, &temp, NULL, NULL, NULL, NULL);
		FD_ZERO(&temp);
	}
	CU_ASSERT_EQUAL(connection->current_state, SEND_RCPT_TO);

	//free

	shutdown(connection->id, 0);
	close(connection->id);
	free(connection->task_pool->path);
	free(connection->task_pool->domain);
	free(connection->task_pool);
	free(connection->domain);
	free(connection);
	FreeDictionary();
	CU_ASSERT_EQUAL(state, 0);
}

int add_to_suit_smtp_test(CU_pSuite suite)
{
	if (suite)
	{
		ADD_SUITE_TEST(suite, domain_is_NULL)
		ADD_SUITE_TEST(suite, bad_domain)
		ADD_SUITE_TEST(suite, connection_is_blocked)
		ADD_SUITE_TEST(suite, greeting_bad_graph_state)
		ADD_SUITE_TEST(suite, in_greeting_not_correct_domain)
		ADD_SUITE_TEST(suite, send_ehlo_and_correct_result)
		ADD_SUITE_TEST(suite, send_ehlo_and_change_state_on_error)
		// ADD_SUITE_TEST(suite, send_and_recv_ehlo_in_receive_not_my_domain)
		// ADD_SUITE_TEST(suite, in_greeting_421_status)
	}
	return CU_get_error();
}