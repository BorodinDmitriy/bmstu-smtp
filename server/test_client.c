#include "../common/header.h"

int main(int argc, char **argv) {
	
	int client_sock_fd = 0;
	char buffer[TEST_CLIENT_BUFFER_SIZE];
	int read_bytes;

	struct sockaddr_in address; 
    struct sockaddr_in serv_addr; 

    char *client_stub = "Hello from client"; 
    
    // создаем файловый дескриптор сокета
    client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock_fd < 0) 
    { 
        perror("socket() failed"); 
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
   
    // установка соединения с сервером
    if (connect(client_sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        perror("connect() failed"); 
        exit(TEST_CLIENT_EXIT_FAILURE);
    } 

    // установили соединение - можно общаться с помощью send/recv
    send(client_sock_fd, client_stub, strlen(client_stub), 0); 
    printf("Hello message sent by client\n"); 
    read_bytes = read(client_sock_fd, buffer, TEST_CLIENT_BUFFER_SIZE ); 
    printf("%s\n", buffer); 

    return 0; 
}
