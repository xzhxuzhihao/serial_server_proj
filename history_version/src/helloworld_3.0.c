#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define NET_PORT 248
#define THE_MAX_CONNECT_AT_SAME 16
#define TCP_UDP SOCK_STREAM
#define IP_FAMIALY AF_INET
void handle_connection(int *client_fd){
	printf("ok\n");
	printf("ok\n");
	printf("ok\n");
	write(*client_fd, "123", 3);
	int fd = open("/dev/ttyACM0", O_WRONLY);
	while(1){
		wirte(fd, "1" , 1);
		usleep(100);
	}
	close(*client_fd);
	close(fd);
	free(client_fd);
}
int main(){
	int server_fd = socket(IP_FAMIALY, TCP_UDP, 0); //SOCK_STREAM for TCP,  SOCK_DGRAM for UDP
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = IP_FAMIALY;
    server_addr.sin_port = htons(NET_PORT); // replace with your port number
    server_addr.sin_addr.s_addr = INADDR_ANY;
    unsigned char num_err = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    int listen_erro = listen(server_fd, THE_MAX_CONNECT_AT_SAME);

    while (1) {
        struct sockaddr_in client_addr = {0};
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = (int *)malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        pthread_t thread;
        int pthread_creat_erro = pthread_create(&thread, NULL, handle_connection, client_fd);
	}
    close(server_fd);
    return 0;
}
