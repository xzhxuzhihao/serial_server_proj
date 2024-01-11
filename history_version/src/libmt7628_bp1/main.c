#include "serial_transfer_api.h"
#include "error.h"
pthread_cond_t cond_main;

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

	int server_fd = socket(IP_FAMIALY, TCP_UDP, 0); //SOCK_STREAM for TCP,  SOCK_DGRAM for UDP
	JUDGE_ERROR_RETURN(server_fd, "main: socket creat error. num 2");
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = IP_FAMIALY;
    server_addr.sin_port = htons(NET_PORT); // replace with your port number
    server_addr.sin_addr.s_addr = INADDR_ANY;
    unsigned char num_err = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	JUDGE_ERROR_RETURN(num_err, "main: Bind error. num 3");
    int listen_erro = listen(server_fd, THE_MAX_CONNECT_AT_SAME);
	JUDGE_ERROR_RETURN(listen_erro, "main: listen function error. num 4");

    while (1) {
        struct sockaddr_in client_addr = {0};
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = (int *)malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		JUDGE_ERROR_RETURN(*client_fd, "main: Accept function error. num 5");

        pthread_t thread;
        int pthread_creat_erro = pthread_create(&thread, NULL, handle_connection, client_fd);
		JUDGE_ERROR_PTHREAD_ERROR_RETURN(pthread_creat_erro, "main: pthread creat erro. num 6");
	}
    close(server_fd);
    return 0;
}
