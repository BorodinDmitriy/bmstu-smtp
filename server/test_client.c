#include "../common/header.h"

void recv_msg_handler() {
   
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

int main(int argc, char **argv) {
	
	int client_sock_fd = 0;
	char buffer[TEST_CLIENT_BUFFER_SIZE];
	int read_bytes;

	//struct sockaddr_in address; 
    struct sockaddr_in serv_addr; 

    //char *client_stub = "Hello from client"; 
    char helo_stub[] = "HELO mysmtpserver.com\r\n" \
    					"QUIT mysmtpserver.com\r\n" ; 

    //char helo_stub[] = "Hello from client"; 
    
    // создаем файловый дескриптор сокета
    client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock_fd < 0) 
    { 
        perror("socket() failed"); 
        exit(TEST_CLIENT_EXIT_FAILURE); 
    } 

    int file_flags = fcntl(client_sock_fd, F_GETFL, 0);
    if (file_flags == -1)
    {
        printf("Fail to receive socket flags");
        exit(TEST_CLIENT_EXIT_FAILURE);
    }

    if (fcntl(client_sock_fd, F_SETFL, file_flags | O_NONBLOCK))
    {	
        printf("Fail to set flag 'O_NONBLOCK' for socket");
        exit(TEST_CLIENT_EXIT_FAILURE); 
    }
   
    // инициализация структуры адреса для подключения к серверу по известному порту
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(SERVER_PORT); 
       
    // преобразование IPv4 and IPv6 адресов из текста в бинарную форму
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)  
    { 
        perror("incorrect address or address is not supported"); 
        exit(TEST_CLIENT_EXIT_FAILURE); 
    } 
   
    fd_set socket_set;
	FD_ZERO(&socket_set);
	FD_SET(client_sock_fd, &socket_set);

	// вечное ожидание соединения по одному из связанных сокетов
	select(client_sock_fd + 1, &socket_set, NULL, NULL, NULL);

	// если так и не дождались возможности писать - ошибка
	if (!FD_ISSET(client_sock_fd, &socket_set)) {
		printf("socket is not available for 15s\n");
		exit(TEST_CLIENT_EXIT_FAILURE);
	} else {
		// установка соединения с сервером
    	if (connect(client_sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    	{ 
        	perror("connect() failed"); 
        	exit(TEST_CLIENT_EXIT_FAILURE);
    	} 


    	FD_ZERO(&socket_set);
		FD_SET(client_sock_fd, &socket_set);

		// вечное ожидание соединения по одному из связанных сокетов
		select(client_sock_fd + 1, &socket_set, NULL, NULL, NULL);

		// если так и не дождались возможности писать - ошибка
		if (!FD_ISSET(client_sock_fd, &socket_set)) {
			printf("socket is not available for 15s\n");
			exit(TEST_CLIENT_EXIT_FAILURE);
		} else {
			// установили соединение - можно общаться с помощью send/recv
    		send(client_sock_fd, helo_stub, strlen(helo_stub), 0); 
    		//read_bytes = read(client_sock_fd, buffer, TEST_CLIENT_BUFFER_SIZE ); 
    		printf("au\n"); 
		}












    	

    	char receive_message[TEST_CLIENT_BUFFER_SIZE] = {};
    	while (1) {
    		FD_ZERO(&socket_set);
			FD_SET(client_sock_fd, &socket_set);

			// вечное ожидание соединения по одному из связанных сокетов
			select(client_sock_fd + 1, &socket_set, NULL, NULL, NULL);

			// если так и не дождались возможности писать - ошибка
			if (!FD_ISSET(client_sock_fd, &socket_set)) {
				printf("socket is not available for 15s\n");
				exit(TEST_CLIENT_EXIT_FAILURE);
			} else {
				int receive = recv(client_sock_fd, receive_message, TEST_CLIENT_BUFFER_SIZE, 0);
        		printf("%d\n",receive);
       		 	if (receive > 0) {
            		printf("ua%s", receive_message);
            		receive_message[3] = '\0';
            		if (STR_EQUAL(receive_message, "221")) {
            			break;
            		}
            		memset(receive_message, 0, TEST_CLIENT_BUFFER_SIZE * sizeof(char));
        		} else if (receive == 0) {
        			printf("bla");
            		break;
        		} else { 
        			printf("ba");
            		break;
        		}
			}


        	
    	}
	}

    


    

    return 0; 
}
