#include "serial_transfer_api.h"
#include "error.h"
pthread_cond_t cond_main;

int main(){
	pthread_t serial_config_thread;
	pthread_mutex_t lock_tmp = PTHREAD_MUTEX_INITIALIZER;
    int pthread_creat_erro = pthread_create(&serial_config_thread, NULL, serial_config_api, NULL);
	JUDGE_ERROR_RETURN(pthread_creat_erro, "main: serial config thread create error. num 1");
	pthread_mutex_lock(&lock_tmp); 
	pthread_cond_wait(&cond_main, &lock_tmp);
	pthread_mutex_unlock(&lock_tmp); 

	pthread_t thread_socket_transfer[96];
	int num_port[96];
	for(int i=0; i<96; i++){
		num_port[i] = i;
		int pthread_creat_erro = pthread_create(&thread_socket_transfer[i], NULL, socket_16_listen_func, &num_port[i]);
		JUDGE_ERROR_PTHREAD_ERROR_RETURN(pthread_creat_erro, "main: pthread creat erro. num 6");
	}
	for(int i=0; i<96; i++)
		pthread_join(thread_socket_transfer[i], NULL);

    return 0;
}
void *socket_16_listen_func(void *arg){
	int *num_port = (int *)arg;
	int net_port = NET_PORT_BASE;
	unsigned char serial_port = (unsigned char)(*num_port/3);
	unsigned char tcp_type = (unsigned char)(*num_port%3);
	net_port += (int)(tcp_type*100 + serial_port);

	int server_fd = socket(IP_FAMIALY, TCP_UDP, 0); //SOCK_STREAM for TCP,  SOCK_DGRAM for UDP
	JUDGE_ERROR_RETURN(server_fd, "main: socket creat error. num 2");
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = IP_FAMIALY;
    server_addr.sin_port = htons(net_port); // replace with your port number
    server_addr.sin_addr.s_addr = INADDR_ANY;
    unsigned char num_err = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	JUDGE_ERROR_RETURN(num_err, "main: Bind error. num 3");
    int listen_erro = listen(server_fd, THE_MAX_CONNECT_AT_SAME);
	JUDGE_ERROR_RETURN(listen_erro, "main: listen function error. num 4");

	while(1){
		serial_transfer socket_serial_transfer;
		struct sockaddr_in client_addr = {0};
		socklen_t client_addr_len = sizeof(client_addr);
		int *client_fd = (int *)malloc(sizeof(int));
		*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		JUDGE_ERROR_RETURN(*client_fd, "main: Accept function error. num 5");
		SEND_MESSAGE_CLIENT("Connected\n", *client_fd);
		socket_serial_transfer.client_fd = client_fd;
		socket_serial_transfer.serial_port = serial_port;
		
		pthread_t thread_serial_transfer;
		int pthread_creat_erro = pthread_create(&thread_serial_transfer, NULL, handle_connection, &socket_serial_transfer);
		JUDGE_ERROR_RETURN(pthread_creat_erro, "main: thread create error. num 6");
	}
	close(server_fd);
	
	return NULL;
}
