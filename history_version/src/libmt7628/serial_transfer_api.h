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

#define NET_PORT_BASE 4000 
#define THE_MAX_CONNECT_AT_SAME 32
#define TCP_UDP SOCK_STREAM
#define IP_FAMIALY AF_INET
#define MESSAGE_NUM 2
#define BUFFER_NUM_REC 1024
#define BUFFER_NUM_SED 1024
#define SERIAL_PARA_SIZE 9 
#define SERIAL_DIRECTION O_RDWR
#define DEVICE_MODE_FILE "/etc/config/device_mode"
#define SERIAL_DEVICE_NAME "/dev/ttyACM"
#define COMMUNICATE_SERIAL_NAME "/dev/ttyS1"
#define NETWORK_FILE_NAME "/etc/config/network"
#define WATCH_SERIAL_NAME "/etc/config/serial_port_para_"
#define MESSAGE_0 "Please input your serial port(0-15): "
#define MESSAGE_1 "Ok, connected. Please input data you want to send, and end by press ctrl-d.\nYour data: "
#define GET_IP_COMMAND "ip addr show dev br-lan | awk '{print $2}' | head -n 3 | tail -n 1 | cut -d '/' -f 1 | tr -s '\n' '.'; ip addr show dev eth0.2 | awk '{print $2}' | head -n 3 | tail -n 1|cut -d '/' -f 1"
#define WAN_TO_LAN_COMMAND "uci set network.@switch_vlan[0].ports='0 2 3 4 6t';uci set network.@switch_vlan[1].ports='1 6t';uci commit network;/etc/init.d/network restart > /dev/null;"
#define LAN_TO_WAN_COMMAND "uci set network.@switch_vlan[0].ports='0 1 3 4 6t';uci set network.@switch_vlan[1].ports='2 6t';uci commit network;/etc/init.d/network restart > /dev/null;"
#define SAVE_MODE_NORMAL_COMMAND "echo -n '0' > /etc/config/device_mode;"
#define SAVE_MODE_CASCADE_COMMAND "echo -n '1' > /etc/config/device_mode;"
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN_FILENAME     (  EVENT_SIZE + 36 ) 
#define BUF_LEN_READ 512
#define SIG_IP_FRAME 172 //OxAC 
#define SIG_PARA_NORMAL_FRAME 163 //0xA3
#define SIG_PARA_CASCADE_FRAME 167 //0xA7
#define SIG_UP_KEY 92 //0x5c
#define SIG_DOWN_KEY 83 //0x53
#define SIG_RESET 0x95
#define DELAY_TIME 2 //ms
#define SERIAL_PARA_QUOTA 39 // ' in serial configuation file
#define NORMAL_MODE 0
#define CASCADE_MODE 1
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
#define SAVE_MODE_PARA_SIZE 256
#define IP_ADDR_CHAR_LENTH 50
#define SERIAL_FILE_BUF_LENTH 512
#define FIXED_IP "192168000149" /* equal to 192.168.0.149 */

#define MESSAGE_COMMUNICATE {\
							 MESSAGE_0,\
							 MESSAGE_1\
							}

#define SEND_MESSAGE_CLIENT(MASSAGE, SEND_FD)	{\
								int send_nums = write(SEND_FD, MASSAGE, strlen(MASSAGE));\
								if(send_nums <= 0){\
									EXIT_SS("Communicate error.");\
								}\
								}

#define GET_SERIAL_NUM_CHAR() char tmp[3] = {0};\
								tmp[0] = (i >= 10)? '0'+i/10:'0'+i;\
								tmp[1] = (i >= 10)? '0'+i%10:'\0';\
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

#define PRINTF_IP_ADDRESS(name) \
				printf("The ip address ");\
				for(int i=0; i<16; i++){\
					if(i%4 == 3)\
						printf("%d ", name[i+1]);\
					else\
						printf("%d.", name[i+1]);\
				}\
				printf("has been send\n");

#define PRINTF_SERIAL_PARA() \
					printf("Serial port %d parameter is %s %s %s %s %s %s\n", arg_serial->seqence, baud_rate, data_bit, parity_bit, stop_bit, flowcontrol, port_type);

#define PRINTF_SERIAL_STANDARD()  \
					pthread_mutex_lock(&lock_serial);\
					printf("The port type para ");\
					for(int i=0; i<64; i++){\
						printf("%c", serial_standard[i]);\
					}\
					printf(" has been send\n");\
					pthread_mutex_unlock(&lock_serial);  

#define PRINTF_SERIAL_COLSED() \
					printf("Serial port %d has been closed\n", socket_serial_transfer->serial_port);

#define LOCK_SOCKET_THREAD_CREATE()    \
				pthread_mutex_t lock_socket_0 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_1 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_2 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_3 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_4 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_5 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_6 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_7 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_8 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_9 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_10 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_11 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_12 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_13 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_14 = PTHREAD_MUTEX_INITIALIZER;\
				pthread_mutex_t lock_socket_15 = PTHREAD_MUTEX_INITIALIZER;

#define	DEFINE_LOCK() switch (serial_port){\
					case 0:\
						lock_socket = &lock_socket_0;\
						break;\
					case 1:\
						lock_socket = &lock_socket_1;\
						break;\
					case 2:\
						lock_socket = &lock_socket_2;\
						break;\
					case 3:\
						lock_socket = &lock_socket_3;\
						break;\
					case 4:\
						lock_socket = &lock_socket_4;\
						break;\
					case 5:\
						lock_socket = &lock_socket_5;\
						break;\
					case 6:\
						lock_socket = &lock_socket_6;\
						break;\
					case 7:\
						lock_socket = &lock_socket_7;\
						break;\
					case 8:\
						lock_socket = &lock_socket_8;\
						break;\
					case 9:\
						lock_socket = &lock_socket_9;\
						break;\
					case 10:\
						lock_socket = &lock_socket_10;\
						break;\
					case 11:\
						lock_socket = &lock_socket_11;\
						break;\
					case 12:\
						lock_socket = &lock_socket_12;\
						break;\
					case 13:\
						lock_socket = &lock_socket_13;\
						break;\
					case 14:\
						lock_socket = &lock_socket_14;\
						break;\
					case 15:\
						lock_socket = &lock_socket_15;\
						break;\
					default:\
						EXIT_SS("main: serial num error");\
						exit(1);\
				}
#define PRINTF_CONNECT_INFO()        \
					switch (tcp_type){\
						case 0:\
							printf("Serial port %d has connected at TCP Row Port %d\n", serial_port, net_port);\
							break;\
						case 1:\
							printf("Serial port %d has connected at TCP Telnet Port %d\n", serial_port, net_port);\
							break;\
						case 2:\
							printf("Serial port %d has connected at TCP SSL Port %d\n", serial_port, net_port);\
							break;\
						default:\
							break;\
					}

#define SEVEN_CONVERT(x) switch(x){\
							case 0:\
								x = 0x7E;\
							case 1:\
								x = 0x30;\
							case 2:\
								x = 0x6D;\
							case 3:\
								x = 0x79;\
							case 4:\
								x = 0x33;\
							case 5:\
								x = 0x5B;\
							case 6:\
								x = 0x5F;\
							case 7:\
								x = 0x70;\
							case 8:\
								x = 0x7F;\
							case 9:\
								x = 0x7B;\
						}	
typedef struct fd_serial_client fd_serial_client;
typedef struct serial_port_info serial_port_info;
typedef struct serial_config serial_config;
typedef struct serial_transfer serial_transfer;
typedef struct wait_time wait_time;
typedef struct occupy_signal_send occupy_signal_send;
typedef struct serial_thread_signal serial_thread_signal;

struct serial_transfer{
	int *client_fd;
	int serial_port;
};
struct fd_serial_client {
	int f_serial_fd;
	int f_client_fd;
	int close_signal;
};
struct share_room{
	unsigned char read_data;
	bool is_read;
};
struct wait_time{
	int wait_bit_per;
	int data_bit;
	int baud_rate_value;
};
struct serial_port_info{
	char device_name[14];
	char watch_serial_name[50];
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
	serial_port_info serial_port[32];
};
struct time_signal{
	bool the_end;
	int fd;
	int num;
	unsigned char *serial_standard;
};
struct occupy_signal_send{
	bool signal;
	int client_fd;
};
struct serial_thread_signal{
	int *close_signal;
	pthread_t *thread;
};

void *socket_16_listen_func(void *arg);
void *handle_connection(void *arg);
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
int char_to_dec(char *char_array, int num);
void *serial_tranfer_occupy(void *arg);
void *serial_thread_close(void *arg);
void *send_communicate_signal_func(void *arg);
int get_device_mode();
void *send_signal_func(void *arg);
