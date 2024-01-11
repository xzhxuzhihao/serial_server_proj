#include "serial_transfer_api.h"
#include "error.h"
pthread_cond_t cond_main;
pthread_mutex_t lock_socket_0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_7 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_8 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_9 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_10 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_11 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_12 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_13 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_14 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_socket_15 = PTHREAD_MUTEX_INITIALIZER;

int main(){
	printf("ok5\n");
	pthread_t serial_config_thread;
	pthread_mutex_t lock_tmp = PTHREAD_MUTEX_INITIALIZER;
    int pthread_creat_erro = pthread_create(&serial_config_thread, NULL, serial_config_api, NULL);
	printf("ok4\n");
	JUDGE_ERROR_RETURN(pthread_creat_erro, "main: serial config thread create error. num 1");
	for(int i=0; i<5; i++){
		printf("ok7\n");
	}
//	while(1){
//		printf("ok7");
//		sleep(1);
//	}
	pthread_mutex_lock(&lock_tmp); // lock the communicate signal
	pthread_cond_wait(&cond_main, &lock_tmp);
	pthread_mutex_unlock(&lock_tmp); // lock the communicate signal
	printf("ok6\n");

	pthread_t thread_socket_transfer[48];
	int num_port[48];
	for(int i=0; i<48; i++){
		num_port[i] = i;
		int pthread_creat_erro = pthread_create(&thread_socket_transfer[i], NULL, socket_16_listen_func, &num_port[i]);
		JUDGE_ERROR_PTHREAD_ERROR_RETURN(pthread_creat_erro, "main: pthread creat erro. num 6");
	}
	for(int i=0; i<48; i++)
		pthread_join(thread_socket_transfer[i], NULL);

    return 0;
}
void *socket_16_listen_func(void *arg){
	serial_transfer socket_serial_transfer;
	int *num_port = (int *)arg;
	int net_port = 4000;
	unsigned char serial_port = (unsigned char)(*num_port/3);
	unsigned char tcp_type = (unsigned char)(*num_port%3);
	net_port += (int)(tcp_type*100 + serial_port);
	socket_serial_transfer.client_fd = NULL;
	socket_serial_transfer.serial_port = serial_port;
	pthread_mutex_t *lock_socket = NULL;
	DEFINE_LOCK();

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

	struct sockaddr_in client_addr = {0};
	socklen_t client_addr_len = sizeof(client_addr);
	int *client_fd = (int *)malloc(sizeof(int));
	*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	JUDGE_ERROR_RETURN(*client_fd, "main: Accept function error. num 5");
	socket_serial_transfer.client_fd = client_fd;
		
	pthread_mutex_lock(lock_socket); // lock
	handle_connection(&socket_serial_transfer);	
	pthread_mutex_unlock(lock_socket); // unlock 
	close(server_fd);
	free(client_fd);
}
