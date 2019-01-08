#include "message.h"

char* get_mail(char *message) {
	char *result = NULL;
	char *start = strchr(message, MAIL_START);
	if (!start)
		return NULL;
	char *end = strchr(start + 1, MAIL_END);
	if (!end)
		return NULL;
	return select_from_message(message, result, start, end);
}

char* get_domain(char* message) {
    char* info = NULL;
    char* start = strstr(message, " ");
    if (start == NULL)
        return NULL;
    char *end = message + strlen(message);
    return select_from_message(message, info, start, end);
}

//void generate_filename(char *seq);
//int save_message(struct msg *message, char *path);

char* ip_to_hostname(char *hostname) {
  	struct hostent *hent;
  	struct in_addr addr;

  	if (!inet_aton(hostname, &addr))
    	return(hostname);

  	if((hent = gethostbyaddr((char *)&(addr.s_addr), sizeof(addr.s_addr),AF_INET)))
     strcpy(hostname, hent->h_name);

  	return hostname;
}

char* select_from_message(char* message, char* buffer, char* start, char* end) {
    if ( start && end ) {
        start++;
        int length = end - start;
        buffer = (char*) malloc(length + 1);
        memcpy(buffer, start, length + 1);
        buffer[length] = '\0';
    }
    return buffer;
}

//int make_dir(char *path);
//char* make_dir_path(char *path, char *address_to);
//char* make_tmp_path(char *path, char *name);
//char* make_new_path(char *path, char *name);