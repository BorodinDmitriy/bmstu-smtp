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

void generate_filename(char *seq) {
	struct timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv,&tz);
    srand(tv.tv_usec);
    sprintf (seq,"%lx.%lx.%x",tv.tv_sec, tv.tv_usec, rand());
}

char* concat_strings(char *s1, char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2);
    result[len1+len2] = '\0';
    return result;
}

int make_dir(char* dir_path) {
    mkdir(dir_path, 0700);
    char* new_dir = concat_strings(dir_path,"/new");
    mkdir(new_dir, 0700);
    free(new_dir);
    new_dir = concat_strings(dir_path,"/cur");
    mkdir(new_dir, 0700);
    free(new_dir);
    new_dir = concat_strings(dir_path,"/tmp");
    mkdir(new_dir, 0700);
    free(new_dir);
    return 1;
}

char* make_dir_path(char *path, char *address_to) {
	char *tmp_maildir = concat_strings(path, address_to);
	struct stat file_stat;
	if (stat(tmp_maildir, &file_stat) == -1)
        mkdir(tmp_maildir, 0700);
    char* dir_path = concat_strings(tmp_maildir,"/maildir");
    if (stat(dir_path, &file_stat) == -1)
        make_dir(dir_path);
    free(tmp_maildir);
    return dir_path;
}

char* make_tmp_path(char* dir_path, char* name) {
    char* tmp_dir = concat_strings(dir_path,"/tmp/");
    char* tmp_file = concat_strings(tmp_dir, name);
    free(tmp_dir);
    return tmp_file;
}

char* make_new_path(char* dir_path, char* name) {
    char* new_dir = concat_strings(dir_path,"/new/");
    char* new_file = concat_strings(new_dir, name);
    free(new_dir);
    return new_file;
}

int save_message(struct msg *message, char *path) {
	if (strcmp(message->from, "") == 0) {
		return ERROR_FROM;
	}
	if (strcmp(message->to[0], "") == 0) {
		return ERROR_TO;
	}

	if (strcmp(message->body, "") == 0) {
		return ERROR_BODY;
	}
	int i;
	for (i = 0; i < message->recepients_num; i++) {
		char *dir_path = make_dir_path(path, message->to[i]);
		char filename[20];
		generate_filename(filename);
		char *tmp_path = make_tmp_path(dir_path, filename);
		char *new_path = make_new_path(dir_path, filename);
		FILE *fp = fopen(tmp_path,"a");
        fprintf(fp,"From: %s\r\n",message->from);
        fprintf(fp,"To: %s\r\n",message->to[i]);
        fprintf(fp,"%s\n",message->body);
        fclose(fp);
        rename(tmp_path,new_path);
        free(tmp_path);
        free(new_path);
        free(dir_path);
	}
	return 0;
}
