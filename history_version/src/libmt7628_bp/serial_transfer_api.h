#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <termios.h>
#include <errno.h>
#include <sys/inotify.h>

#define NET_PORT 248
#define THE_MAX_CONNECT_AT_SAME 16
#define TCP_UDP SOCK_STREAM
#define IP_FAMIALY AF_INET
#define MESSAGE_NUM 2
#define BUFFER_NUM_REC 1024
#define BUFFER_NUM_SED 1024
#define SERIAL_PARA_SIZE 5
#define SERIAL_DIRECTION O_RDWR
#define SERIAL_DEVICE_NAME "/dev/ttyACM"
#define COMMUNICATE_SERIAL_NAME "/dev/ttyS0"
#define NETWORK_FILE_NAME "/etc/config/network"
#define WATCH_SERIAL_NAME "/tmp/test_serial_para_"
#define MESSAGE_0 "Please input your serial port(0-15): "
#define MESSAGE_1 "Ok, connected. Please input data you want to send, and end by press ctrl-d.\nYour data: "
#define GET_IP_COMMAND "ip addr show dev br-lan | awk '{print $2}' | head -n 3 | tail -n 1 | cut -d '/' -f 1 ; ip addr show dev eth0.2 | awk '{print $2}' | head -n 3 | tail -n 1|cut -d '/' -f 1"
#define WAN_TO_LAN_COMMAND "uci set network.@switch_vlan[0].ports='0 2 3 4 6t';uci set network.@switch_vlan[1].ports='1 6t';uci commit network;/etc/init.d/network restart;"
#define LAN_TO_WAN_COMMAND "uci set network.@switch_vlan[0].ports='0 1 3 4 6t';uci set network.@switch_vlan[1].ports='2 6t';uci commit network;/etc/init.d/network restart;"
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN_FILENAME     (  EVENT_SIZE + 36 ) 
#define BUF_LEN_READ 40
#define SIG_IP_FRAME 172 //OxAC 
#define SIG_PARA_FRAME 163 //0xA3
#define SIG_UP_KEY 92 //0x5c
#define SIG_DOWN_KEY 83 //0x53
#define DELAY_TIME 2 //ms
#define SERIAL_PARA_QUOTA 39 // ' in serial configuation file
#define BAUD_RATE_1200 B1200
#define BAUD_RATE_2400 B2400
#define BAUD_RATE_4800 B4800
#define BAUD_RATE_9600 B9600
#define BAUD_RATE_19200 B19200
#define BAUD_RATE_38400 B38400
#define BAUD_RATE_57600 B57600
#define BAUD_RATE_115200 B115200
#define BAUD_RATE_230400 B230400
#define BAUD_RATE_921600 B921600
#define FLOW_MODE_NONE "NONE"
#define FLOW_MODE_SOFT "SOFT"
#define FLOW_MODE_HARD "HARD"
#define SERIAL_MODE_RS232 "RS232"
#define SERIAL_MODE_RS422 "RS422"
#define SERIAL_MODE_RS485 "RS485"
#define IP_COMMAND_SIZE 256
#define WAN_LAN_COMMAND_SIZE 256
#define IP_ADDR_CHAR_LENTH 50
#define SERIAL_FILE_BUF_LENTH 512

#define MESSAGE_COMMUNICATE {\
							 MESSAGE_0,\
							 MESSAGE_1\
							}
#define SEND_MESSAGE_CLIENT(MASSAGE)	{\
								int send_nums = write(*client_fd, MASSAGE, strlen(MASSAGE));\
								if(send_nums <= 0){\
									EXIT_SS("Communicate error.");\
									return NULL;\
								}\
								}
#define GET_SERIAL_NUM_CHAR() char tmp[3] = {0};\
							tmp[0] = (i >= 10)? '1':'0'+i;\
							tmp[1] = (i >= 10)? '0'+i-10:'\0';\
							tmp[2] = '\0';
#define JUDGE_THE_INIT_PRO() while(1){\
								int i=0;\
								for(; i<16; i++){\
									if(serial_config_a->serial_port[i].up_power)\
										break;\
								}\
								if(i == 16)\
									break;\
								else\
									continue;\
							 }

typedef struct fd_serial_client fd_serial_client;
typedef struct serial_port_info serial_port_info;
typedef struct serial_config serial_config;
typedef struct signal_init signal_init;

struct fd_serial_client {
	int f_serial_fd;
	int f_client_fd;
};
struct share_room{
	unsigned char read_data;
	bool is_read;
};
struct serial_port_info{
	char device_name[14];
	char watch_serial_name[30];
	int fd;   //description id
	int fd_w;   //watch description id
	int wd;		//add watch id
	int baud_rate;
	int seqence;
	bool up_power; // Upper power status
	unsigned char stop_bit;  //1 for 1 bit, 2 for 2 bit
	unsigned char parity_bit; //0 for none, 1 for odd, 2 for enen
							  
	char *serial_standard; //port_type parameter
};
struct serial_config{
	serial_port_info serial_port[16];
};
struct time_signal{
	bool the_end;
	int fd;
	int num;
	unsigned char *serial_standard;
};
struct signal_init{
	int init_signal;
	pthread_cond_t cond_main;
};

void *handle_connection(void *client_fd);
char *communicate_client(int *client_fd);
unsigned char check_string_end(char *check_str, unsigned char char_num);
void* read_data_from_serial(void *s_c_fd);
void* write_data_to_serial(int *client_fd, int *serial_fd);
void* serial_config_api(void* arg);
void *communicate_send_seiral_standard(void *arg);
void* communicate_ip_addr_send(void* arg);
int communicate_send_is_success(struct time_signal* signal, unsigned char signal_buffer);
void* communicate_read_data(void *arg);
unsigned char* communicate_read_ipaddr();
unsigned char* str_to_num_ipaddr(char *ipaddr_char);
void* right_key_process_func(void* arg);
void* left_key_process_func(void* arg);
void* resend_time_func(void *arg);
unsigned char *str_to_num(char *serial_standard);
void *thread_func(void *arg);
void serial_para_proc(serial_port_info *arg_serial, char *buffer, struct termios *tty);
