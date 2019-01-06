#ifndef MESSAGE_H
#define MESSAGE_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAIL_START '<'
#define MAIL_END '>'
#define DOMAIN_START ' '
#define DOMAIN_END '\0'

#define ERROR_TO -1
#define ERROR_FROM -2
#define ERROR_BODY -3
#define ERROR_NUM -4

struct msg {
	char **to;
	char *from;
	char *body;
	int recepients_num;
};

//char* get_mail(char *message);
char* get_domain(char *message);
//void generate_filename(char *seq);
//int save_message(struct msg *message, char *path);
char* ip_to_hostname(char *hostname);
char* select_from_message(char *message, char *buffer, char *start, char *end);

//int make_dir(char *path);
//char* make_dir_path(char *path, char *address_to);
//char* make_tmp_path(char *path, char *name);
//char* make_new_path(char *path, char *name);

#endif