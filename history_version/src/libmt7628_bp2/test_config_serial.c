#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_tmp = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond;

void* thread_func(void *arg) {
	int afi = 5;
	switch (afi){
		case 3:
			printf("bbb\n");
			break;
		case 5:
			printf("ccc\n");
			break;
	}
    pthread_mutex_lock(&lock_tmp); // Wait for the lock
	int * arg_a =(int *)arg;				   
	*arg_a += 5;
    printf("Thread ID %ld is reading from /dev/ttyS0\n", pthread_self());
	printf("%d \n", *(int *)arg);
    // Implement your logic to read from /dev/ttyS0 here.
	sleep(2);
	pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock_tmp); // Wait for the lock
	for(int i=0; i<3; i++){
		if(*arg_a == 5){
			printf("ok\n");
			sleep(1);
		}else{
			printf("bad\n");
			sleep(1);
		}
	}

    return NULL;
}
pthread_mutex_t lock_a;
pthread_mutex_t lock_b;
void * do_work(void *arg);
int communicate_signal = 0;
void * do_work(void *arg){
	pthread_mutex_lock(&lock_b);
	communicate_signal++;
	int *return_val = (int *)malloc(sizeof(int));
	*return_val = 1;
	int fd = open("/tmp/test", O_RDWR);
	char buf[10];
	read(fd, buf, 4);
	printf("%s\n", buf);
	pthread_mutex_unlock(&lock_b);
	return return_val;
}
struct aa_b{
	int al;
};
int aa_b = 19999;
#define wf int af = 1312;\
			int bf = 133;\
			int cf = 144;
			
int aa_c = 3;
int main() {
	int c_c = 15;
	char c_a[5];
	sprintf(c_a, "%d", c_c);
	printf("c_a = %s\n", c_a);
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_RR);
	sched_setscheduler(0, SCHED_RR, &param);
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 1000000L;
	int fd_ss = open("/dev/ttyS0", O_RDWR|O_NOCTTY|O_SYNC);
    pthread_t thread0, thread1, thread2,thread3, thread4, thread5, thread6, thread7, thread8,thread9, thread10,thread11, thread12, thread13, thread14, thread15;
	int fd = open("/tmp/test", O_RDWR);
	unsigned char buf;
	read(fd, &buf, 1);
	wf;
	printf("%d\n", aa_b);
	FILE *fd_d;
	char buf_d[1024];
	char cmd[1024];
	char *ipaddr = (char *)malloc(sizeof(char)*1024);
//	snprintf(cmd, sizeof(cmd), "ip addr show dev br-lan | awk '{print $2}' | head -n 3 | tail -n 1 | cut -d '/' -f 1 | tr -d '\n'; ip addr show dev eth0.2 | awk '{print $2}' | head -n 3 | tail -n 1|cut -d '/' -f 1 | tr -d '\n'");	
//	fd_d = popen(cmd, "r");
//	fgets(buf_d, 1024, fd_d);
//	pclose(fd_d);
//	printf("%s\n", buf_d);
	char command[256];
	snprintf(command, sizeof(command), "swconfig dev switch0 vlan 1 set ports '0 2 3 4 6t';\
			swconfig dev switch0 vlan 2 set ports '1 6t';\
			swconfig dev switch0 set apply");
	int *return_val = (int *)malloc(sizeof(int));
	*return_val = 1;
	pthread_create(&thread0, NULL, thread_func,return_val);
    pthread_mutex_lock(&lock); // Wait for the lock
	pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock); // Release the lock
	for(int i=0; i<10; i++){
		printf("okok\n");
		write(fd_ss, "1111", 4);
		fdatasync(fd_ss);
		nanosleep(&ts, NULL);
	}
	pthread_join(thread0, NULL);
	for(int i=0; i<5; i++){
		printf("first, %d\n", 0xA3);
		sleep(1);
	}
	printf("%d\n", communicate_signal);
	while(1);
	printf("all OK\n");
    return 0;
}

