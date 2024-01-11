#include "serial_transfer_api.h"
#include "error.h"
#include <string.h>

struct share_room communicate_share_room = {0, 1};  //1 for having read
pthread_mutex_t lock_serial = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_connection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_tmp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_communicate_signal = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_communicate_signal_data = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_communicate_share_room = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_ip_address = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond;
extern pthread_cond_t cond_main;
wait_time transfer_wait_time;
int communicate_signal = 0;
int device_mode = 0; // 0 for normal; 1 for connection
int send_signal = 1;
char pre_serial_standard[64];

void serial_para_proc(serial_port_info *arg_serial, char *buffer, struct termios *tty){
	char baud_rate[7];	
	char stop_bit[2];
	char parity_bit[2];
	char data_bit[2]; 
	char flowcontrol[5]; 
	char port_type[6];
	int type = 0;
	
	tcgetattr(arg_serial->fd, tty);
	char words[20] = {0};
	for(int i=0; i<SERIAL_FILE_BUF_LENTH&&type<6; i++){
		int j = 0;
		if(buffer[i] == ' '){
			i++;
			while(buffer[i] != SERIAL_PARA_QUOTA && buffer[i] != '\n' && buffer[i+1] != ' '){
				words[j++] = buffer[i++];
			}
			if(buffer[i+1] == ' '){
				words[j++] = buffer[i];
				words[j] = '\0';

			}
		}
	    if(buffer[i] == SERIAL_PARA_QUOTA && !strcmp(words, "speed")){
			i++;
			int j=0;
			for(; j<6; j++, i++){
				if(buffer[i] >= '0' && buffer[i] <= '9')
					baud_rate[j] = buffer[i];
				else 
					break;
			}
			baud_rate[j] = '\0';
			if(!strcmp(baud_rate, "1200"))
				cfsetospeed(tty, BAUD_RATE_1200);
			else if(!strcmp(baud_rate, "2400"))
				cfsetospeed(tty, BAUD_RATE_2400);
			else if(!strcmp(baud_rate, "4800"))
				cfsetospeed(tty, BAUD_RATE_4800);
			else if(!strcmp(baud_rate, "9600"))
				cfsetospeed(tty, BAUD_RATE_9600);
			else if(!strcmp(baud_rate, "19200"))
				cfsetospeed(tty, BAUD_RATE_19200);
			else if(!strcmp(baud_rate, "38400"))
				cfsetospeed(tty, BAUD_RATE_38400);
			else if(!strcmp(baud_rate, "57600"))
				cfsetospeed(tty, BAUD_RATE_57600);
			else if(!strcmp(baud_rate, "115200"))
				cfsetospeed(tty, BAUD_RATE_115200);
			else if(!strcmp(baud_rate, "230400"))
				cfsetospeed(tty, BAUD_RATE_230400);
			else if(!strcmp(baud_rate, "921600"))
				cfsetospeed(tty, BAUD_RATE_921600);
			type += 1;
		}else if(buffer[i] == SERIAL_PARA_QUOTA && !strcmp(words, "databit")){
			i++;
			int j=0;
			data_bit[j++] = buffer[i++];
			data_bit[j] = '\0';
			if(!strcmp(data_bit, "8"))
				(*tty).c_cflag |= CS8; // 8-bit data
			else if(!strcmp(data_bit, "7"))
				(*tty).c_cflag |= CS7;
			else if(!strcmp(data_bit, "6"))
				(*tty).c_cflag |= CS6;
			type += 1;
		}else if(buffer[i] == SERIAL_PARA_QUOTA && !strcmp(words, "parity")){
			i++;
			int j=0;
			parity_bit[j++] = buffer[i++];
			parity_bit[j] = '\0';
			if(!strcmp(parity_bit, "N"))
				(*tty).c_cflag &= ~PARENB;	
			else if(!strcmp(parity_bit, "O")){
				(*tty).c_cflag |= PARENB;
				(*tty).c_cflag |= PARODD;
			}else if(!strcmp(parity_bit, "E")){
				(*tty).c_cflag |= PARENB;
				(*tty).c_cflag &= ~PARODD;
			}
			type++;
		}else if(buffer[i] == SERIAL_PARA_QUOTA && !strcmp(words, "stopbit")){
			i++;
			int j=0;
			stop_bit[j++] = buffer[i++];
			stop_bit[j] = '\0';
			if(!strcmp(stop_bit, "1"))
				(*tty).c_cflag &= ~CSTOPB;
			else if(!strcmp(stop_bit, "2"))
				(*tty).c_cflag |= CSTOPB;
			type++;
		}else if(buffer[i] == SERIAL_PARA_QUOTA && !strcmp(words, "flowcontrol")){
			i++;
			int j=0;
			for(; j<4; j++, i++){
				flowcontrol[j] = buffer[i];
			}
			flowcontrol[j] = '\0';
			if(!strcmp(flowcontrol, FLOW_MODE_NONE)){
				(*tty).c_iflag &= ~(IXON | IXOFF | IXANY); // Disable XON/XOFF flow control on output & input
				(*tty).c_cflag &= ~CRTSCTS; // Disable RTS/CTS (hardware) flow control 
			}else if(!strcmp(flowcontrol, FLOW_MODE_SOFT)){
				(*tty).c_iflag |= (IXON | IXOFF); // Enable XON/XOFF flow control on output & input
			}else if(!strcmp(flowcontrol, FLOW_MODE_HARD)){
				(*tty).c_cflag |= CRTSCTS; // Enable RTS/CTS (hardware) flow control 
			}
			type++;
		}else if(buffer[i] == SERIAL_PARA_QUOTA && !strcmp(words, "portmode")){
			i++;
			int j=0;
			for(; j<5; j++, i++){
				port_type[j] = buffer[i];
			}
			port_type[j] = '\0';
			pthread_mutex_lock(&lock_serial);  // lock the parameter
			if(!strcmp(port_type,SERIAL_MODE_RS232)){
				arg_serial->serial_standard[arg_serial->seqence*2] = '0';	
				arg_serial->serial_standard[arg_serial->seqence*2+1] = '0';
			}else if(!strcmp(port_type, SERIAL_MODE_RS422)){
				arg_serial->serial_standard[arg_serial->seqence*2] = '0';	
				arg_serial->serial_standard[arg_serial->seqence*2+1] = '1';
			}else if(!strcmp(port_type, SERIAL_MODE_RS485)){
				arg_serial->serial_standard[arg_serial->seqence*2] = '1';	
				arg_serial->serial_standard[arg_serial->seqence*2+1] = '0';
			}
			pthread_mutex_unlock(&lock_serial);

			type++;
		}
	}
	if(arg_serial->up_power == 0){
		pthread_mutex_lock(&lock_communicate_signal);
		for(int i=0; i<64; i++){
			if(pre_serial_standard[i] == arg_serial->serial_standard[i]){
				continue;
			}else{
				communicate_signal = 1;
				for(int j=0; j<64; j++){
					pre_serial_standard[j] = arg_serial->serial_standard[j];
				}
				break;
			}
		}
		if(communicate_signal == 1 && send_signal == 1){
			send_signal = 0;
			pthread_mutex_unlock(&lock_communicate_signal);
			sleep(2);
			pthread_mutex_lock(&lock_tmp);
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&lock_tmp);
			communicate_signal = 0;
			send_signal = 1;
			pthread_mutex_lock(&lock_communicate_signal);
		}
		pthread_mutex_unlock(&lock_communicate_signal);
	}
	
	int baud_rate_value = char_to_dec(baud_rate, strlen(baud_rate));
	int stop_bit_value = char_to_dec(stop_bit, strlen(stop_bit));
	int parity_bit_value = 0;
	if(parity_bit[0] == 'N' || parity_bit[0] == 'E'){
		parity_bit_value = 1;
	}
	transfer_wait_time.wait_bit_per = parity_bit_value+stop_bit_value+1;
	transfer_wait_time.baud_rate_value = baud_rate_value;
	transfer_wait_time.data_bit = char_to_dec(data_bit, strlen(data_bit));
	PRINTF_SERIAL_PARA();

	(*tty).c_lflag &= ~ECHO; // Turn off echco
	(*tty).c_oflag &= ~ONLCR; //disable convert '\n' to '\r\n' on output
	(*tty).c_iflag &= ~ICRNL; //disable convert '\r' to '\n'  on input
	tcsetattr(arg_serial->fd, TCSANOW, tty);
}
void *send_signal_func(void *arg){
	while(communicate_signal != 0){
		communicate_signal = 0;
		usleep(10000);
		pthread_mutex_lock(&lock_tmp);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock_tmp);
	}
	send_signal = 1;
}
unsigned char *str_to_num(char *serial_standard){
	unsigned char *send_data = (unsigned char *)malloc(sizeof(unsigned char)*8);;
	int sum = 0;
	int tmp = 1;
	int seq = 0;
	for(int i=0; i<64; i++){
		sum += (serial_standard[7-i+seq*8]-'0')*tmp;	
		tmp *= 2;
		if(0 == (i+1)%8){
			send_data[seq/2+1] = sum;
			seq += 2;
			sum = 0;
			tmp = 1;
		}
	}
	return send_data;
}
void *thread_func(void *arg){
	serial_port_info *arg_serial = (serial_port_info *)arg;
	struct termios tty;
	char buffer_filename[BUF_LEN_FILENAME];
	char buffer_read[BUF_LEN_READ];
	if(arg_serial->up_power == 1){
		int fd = open(arg_serial->watch_serial_name, O_RDWR | O_NOCTTY | O_SYNC);
		int read_error = read(fd, buffer_read, BUF_LEN_READ-1);
		JUDGE_ERROR_RETURN_NULL(read_error, "Serial_config: read error. num 19");
		serial_para_proc(arg_serial, buffer_read, &tty);	
		arg_serial->up_power = 0;
		close(fd);
	}
	while(1){
		time_t last_event_time = time(NULL);
		arg_serial->fd_w = inotify_init();
		JUDGE_ERROR_RETURN_NULL(arg_serial->fd_w, "Serial_config_api:inotify_init error. num 8");
		arg_serial->wd = inotify_add_watch(arg_serial->fd_w, arg_serial->watch_serial_name, IN_MODIFY);
		JUDGE_ERROR_RETURN_NULL(arg_serial->wd, "Serial_config_api:inotify_add_watch error. num 9");
		int length = read(arg_serial->fd_w, buffer_filename, BUF_LEN_FILENAME);
		time_t current_event_time = time(NULL);
		if(current_event_time - last_event_time < 1)
			continue;
		if(length > 0){
			int fd = open(arg_serial->watch_serial_name, O_RDWR | O_NOCTTY | O_SYNC);
			int read_error = read(fd, buffer_read, BUF_LEN_READ-1);

			JUDGE_ERROR_RETURN_NULL(read_error, "Serial_config: read error. num 20");
			serial_para_proc(arg_serial, buffer_read, &tty);	
			close(fd);
		}
	}
}
void* resend_time_func(void *arg){
	struct time_signal* time_signal = (struct time_signal*)arg;	
	while(1){
		usleep(5000);
		if(!time_signal->the_end){
			unsigned char reset = SIG_RESET;
			int resend_error = write(time_signal->fd, &reset, 1); //send reset command
			JUDGE_ERROR_RETURN_NULL(resend_error, "Serial_config: resend write error. num 18_1")
			usleep(104*10); //wait for sending
			usleep(10000); //wait for reseting
			resend_error = write(time_signal->fd, time_signal->serial_standard, time_signal->num);
			if(time_signal->serial_standard[0] == SIG_IP_FRAME){
				usleep(104*10*time_signal->num);
			}
			else if(time_signal->serial_standard[0] == SIG_PARA_NORMAL_FRAME || time_signal->serial_standard[0] == SIG_PARA_CASCADE_FRAME){
				usleep(104*10*time_signal->num);
			}
			JUDGE_ERROR_RETURN_NULL(resend_error, "Serial_config: resend write error. num 18")
		}else{
			break;
		}
	}
}
void* left_key_process_func(void* arg){
	pthread_mutex_lock(&lock_ip_address);
	device_mode = NORMAL_MODE;
	char command_1[WAN_LAN_COMMAND_SIZE];
	char command_2[SAVE_MODE_PARA_SIZE];
	snprintf(command_1, sizeof(command_1), LAN_TO_WAN_COMMAND);
	snprintf(command_2, sizeof(command_2), SAVE_MODE_NORMAL_COMMAND);
	int key_error = system(command_1);
	JUDGE_ERROR_RETURN_NULL(key_error, "Serial_config: system command execute error. num 14");
	key_error = system(command_2);
	JUDGE_ERROR_RETURN_NULL(key_error, "Serial_config: system command execute error. num 14_1");
	sleep(10);
	pthread_mutex_unlock(&lock_ip_address);
	printf("The device mode is normal\n");
}
void* right_key_process_func(void* arg){
	pthread_mutex_lock(&lock_ip_address);
	device_mode = CASCADE_MODE;
	char command_1[WAN_LAN_COMMAND_SIZE];
	char command_2[SAVE_MODE_PARA_SIZE];
	snprintf(command_1, sizeof(command_1), WAN_TO_LAN_COMMAND);
	snprintf(command_2, sizeof(command_2), SAVE_MODE_CASCADE_COMMAND);
	int key_error = system(command_1);
	JUDGE_ERROR_RETURN_NULL(key_error, "Serial_config: system command execute error. num 15");
	key_error = system(command_2);
	JUDGE_ERROR_RETURN_NULL(key_error, "Serial_config: system command execute error. num 15_1");
	sleep(10);
	pthread_mutex_unlock(&lock_ip_address);
	printf("The device mode is CASCADE\n");
}
unsigned char* str_to_num_ipaddr(char *ipaddr_char){
	unsigned char* ipaddr = (unsigned char*)malloc(sizeof(unsigned char)*13);
	int sum=0;
	int tmp=1;
	int num=0;
	for(int i=0; ipaddr_char[i]!='\0'&&num!=9; i++){
		if(ipaddr_char[i] == '.' || ipaddr_char[i] == '\n'){
			for(int j=i-1; j>=0&&ipaddr_char[j]!='.'&&ipaddr_char[j]!='\n';j--){
				sum += (ipaddr_char[j]-'0')*tmp;
				tmp *= 10;
			}
			ipaddr[num+1] = sum;
			sum = 0;
			tmp = 1;
			num++;
		}
	}
	for(int i=0; i<4; i++){
		ipaddr[9+i] = ipaddr[i+1];
	}

	return ipaddr;
}
unsigned char* communicate_read_ipaddr(){
	pthread_mutex_lock(&lock_ip_address);
	unsigned char* ipaddr;
	char ipaddr_char[IP_ADDR_CHAR_LENTH];
	char cmd[IP_COMMAND_SIZE];
	FILE *fp;
	snprintf(cmd, sizeof(cmd), GET_IP_COMMAND);	
	fp = popen(cmd, "r");
	JUDGE_ERROR_NULL_RETURN_NULL(fp, "Serial_config: popen error. num 19");
	fgets(ipaddr_char, IP_ADDR_CHAR_LENTH, fp);
	pclose(fp);
	ipaddr = str_to_num_ipaddr(ipaddr_char);	
	pthread_mutex_unlock(&lock_ip_address);

	return ipaddr;
}
void* communicate_read_data(void *arg){
	unsigned char buffer;
	int fd = open(COMMUNICATE_SERIAL_NAME, O_RDONLY|O_NOCTTY);
	while(1){
		int communicate_read_error = read(fd, &buffer, 1);
		pthread_mutex_lock(&lock_communicate_share_room);
		communicate_share_room.read_data = buffer;
		JUDGE_ERROR_RETURN_NULL(communicate_read_error, "Serial_config: communicate error. num 11");
		communicate_share_room.is_read = 0;
		if(communicate_share_room.read_data == SIG_UP_KEY || communicate_share_room.read_data == SIG_DOWN_KEY){
			communicate_share_room.is_read = 1;
			pthread_t key_process;
			if(communicate_share_room.read_data == SIG_UP_KEY){
				{
					pthread_mutex_lock(&lock_connection);
					int fd = open(COMMUNICATE_SERIAL_NAME, O_WRONLY|O_NOCTTY|O_SYNC);
					char identify = SIG_UP_KEY;
					int write_error = write(fd, &identify, 1);
					JUDGE_ERROR_RETURN(write_error, "Serial_config: communicate write error. num 12_1");
					close(fd);
					pthread_mutex_unlock(&lock_connection);
				}
				int communicate_pthread_error = pthread_create(&key_process, NULL, left_key_process_func, NULL);
				JUDGE_ERROR_RETURN_NULL(communicate_pthread_error, "Serial_config: pthread error. num 12");
			}else{
				{
					pthread_mutex_lock(&lock_connection);
					int fd = open(COMMUNICATE_SERIAL_NAME, O_WRONLY|O_NOCTTY|O_SYNC);
					char identify = SIG_DOWN_KEY;
					int write_error = write(fd, &identify, 1);
					JUDGE_ERROR_RETURN(write_error, "Serial_config: communicate write error. num 13_1");
					close(fd);
					pthread_mutex_unlock(&lock_connection);
				}
				int communicate_pthread_error = pthread_create(&key_process, NULL, right_key_process_func, NULL);
				JUDGE_ERROR_RETURN_NULL(communicate_pthread_error, "Serial_config: pthread error. num 13");
			}
		}
		pthread_mutex_unlock(&lock_communicate_share_room);
	}
	close(fd);
}
int communicate_send_is_success(struct time_signal* signal, unsigned char signal_buffer){
	unsigned char buffer;
	pthread_t resend_time;
	pthread_mutex_lock(&lock_connection);
	int write_error = write(signal->fd, signal->serial_standard, signal->num);
	if(signal->serial_standard[0] == SIG_IP_FRAME)
		usleep(104*10*signal->num);
	else if(signal->serial_standard[0] == SIG_PARA_NORMAL_FRAME || signal->serial_standard[0] == SIG_PARA_CASCADE_FRAME)
		usleep(104*10*signal->num);

	JUDGE_ERROR_RETURN(write_error, "Serial_config: communicate write error. num 16");
	int pthread_error = pthread_create(&resend_time, NULL, resend_time_func, signal);
	JUDGE_ERROR_RETURN(pthread_error, "Serial_config: pthread create error. num 17");
	while(1){
		pthread_mutex_lock(&lock_communicate_share_room); // lock the share room
		if(!communicate_share_room.is_read){
			communicate_share_room.is_read = 1; // 0 for not read, 1 for read
			buffer = communicate_share_room.read_data;
			if(buffer == signal_buffer){
				signal->the_end = 1;
				pthread_mutex_unlock(&lock_communicate_share_room);
				break;
			}
		}
		pthread_mutex_unlock(&lock_communicate_share_room);
		usleep(5000);
	}
	pthread_join(resend_time, NULL);
	signal->the_end = 0;
	usleep(5000); //two message should have 5ms interval
	pthread_mutex_unlock(&lock_connection);
	return 0;
}
unsigned char* convert_to_send_addr(unsigned char *ip_addr){
	unsigned char *send_ip_addr = (unsigned char *)malloc(sizeof(unsigned char)*49);
	unsigned char ip_1[12];
	for(int i=0; i<4; i++){
		int tmp = 1;
		int num = ip_addr[i];
		for(int j=0; j<3; j++){
			tmp = num%10;	
			num /= 10;	
			/* convert tmp to the number of seven segment type */
			SEVEN_CONVERT(tmp);
			ip_1[j+i*3] = tmp;
		}
	}
	memcpy(send_ip_addr+1, ip_1, 12);
	/* Lan ip address */
	unsigned char ip_2[12];
	for(int i=0; i<4; i++){
		int tmp = 1;
		int num = ip_addr[i+4];
		for(int j=0; j<3; j++){
			tmp = num%10;	
			num /= 10;	
			/* convert tmp to the number of seven segment type */
			SEVEN_CONVERT(tmp);
			ip_1[j+i*3] = tmp;
		}
	}
	/* two same wan ip address */
	memcpy(send_ip_addr+13, ip_2, 12);
	memcpy(send_ip_addr+25, ip_2, 12);
	/* the fixed ip address */
	unsigned char ip_3_t[13] = FIXED_IP;	
	unsigned char ip_3[12];	
	for(int i=0; i<12; i++){
		int tmp = ip_3_t[i]-'0';
		SEVEN_CONVERT(tmp);
		ip_3[i] = tmp;
	}
	memcpy(send_ip_addr+37, ip_3, 12);

	return send_ip_addr;
}
void* communicate_ip_addr_send(void *arg){
	unsigned char *ip_addr;
	unsigned char * send_ip_addr;
	int fd = open(COMMUNICATE_SERIAL_NAME, O_WRONLY|O_NOCTTY|O_SYNC);
	char buffer_read[BUF_LEN_READ];
	struct time_signal signal;
	signal.fd = fd;
	signal.num = 49;
	signal.the_end = 0;
	ip_addr = communicate_read_ipaddr();
	ip_addr[0] = SIG_IP_FRAME;
	send_ip_addr = convert_to_send_addr(ip_addr);
	send_ip_addr[0] = SIG_IP_FRAME;
	signal.serial_standard = send_ip_addr;

	PRINTF_IP_ADDRESS(ip_addr);
	communicate_send_is_success(&signal, SIG_IP_FRAME);
	free(ip_addr);
	free(send_ip_addr);
	ip_addr = NULL;
	send_ip_addr = NULL;

	int fd_w = inotify_init();
	int wd = inotify_add_watch(fd_w, NETWORK_FILE_NAME, IN_MODIFY);
	JUDGE_ERROR_RETURN_NULL(wd, "Serial_config: watch error. num 18_1");
	while(1){
		int read_len = read(fd_w, buffer_read, BUF_LEN_READ-1);
		if(read_len > 0){
			ip_addr = communicate_read_ipaddr();	
			ip_addr[0] = SIG_IP_FRAME;
			send_ip_addr = convert_to_send_addr(ip_addr);
			send_ip_addr[0] = SIG_IP_FRAME;
			signal.serial_standard = send_ip_addr;
			PRINTF_IP_ADDRESS(ip_addr);
			communicate_send_is_success(&signal, SIG_IP_FRAME);
			free(ip_addr);
			free(send_ip_addr);
			ip_addr = NULL;
			send_ip_addr = NULL;
		}
		JUDGE_ERROR_RETURN_NULL(read_len, "Serial_config: read error. num 18");
	}
	close(fd);
}
void *send_communicate_signal_func(void *arg){
	while(1){
		usleep(200);
		if(communicate_signal == 1){
			usleep(5000);
			pthread_cond_signal(&cond);
			communicate_signal = 0;
			break;
		} 
	}
	return 0;
}
int get_device_mode(){
	char mode_buf[1];
	int fd = open(DEVICE_MODE_FILE, O_RDONLY);
	int error_judge = read(fd, mode_buf, 1);
	JUDGE_ERROR_RETURN_NULL(error_judge, "Serial_config: pthread_create error. num 22");
	close(fd);
	if(mode_buf[0] == '1')
		return 1;
	else if(mode_buf[0] == '0')
		return 0;
	return 0;
}
void *communicate_send_seiral_standard(void *arg){
	char *serial_standard = (char *)arg;
	int fd = open(COMMUNICATE_SERIAL_NAME, O_WRONLY|O_NOCTTY|O_SYNC);
	unsigned char *serial_standard_num;
	struct time_signal signal;	
	signal.fd = fd;
	/* The size of sending package */
	signal.num = SERIAL_PARA_SIZE;
	signal.the_end = 0; // the end signal of communicate, 1 for end

	usleep(30000);
	for(int i=0; i<64; i++){
		pre_serial_standard[i] = serial_standard[i];
	}
	serial_standard_num = str_to_num(serial_standard);
	if(device_mode == 0)
		serial_standard_num[0] = SIG_PARA_NORMAL_FRAME;
	else if(device_mode == 1)
		serial_standard_num[0] = SIG_PARA_CASCADE_FRAME;
	signal.serial_standard = serial_standard_num;
	PRINTF_SERIAL_STANDARD();
	if(device_mode == NORMAL_MODE)
		communicate_send_is_success(&signal, SIG_PARA_NORMAL_FRAME);
	else
		communicate_send_is_success(&signal, SIG_PARA_CASCADE_FRAME);
	free(signal.serial_standard);
	
	pthread_mutex_lock(&lock_tmp); // tmp lock for wait function, no use for now
	while(1){
		pthread_cond_wait(&cond, &lock_tmp);
		serial_standard_num = str_to_num(serial_standard);
		if(device_mode == 0)
			serial_standard_num[0] = SIG_PARA_NORMAL_FRAME;
		else if(device_mode == 1)
			serial_standard_num[0] = SIG_PARA_CASCADE_FRAME;
		signal.serial_standard = serial_standard_num;
		PRINTF_SERIAL_STANDARD();
		if(device_mode == NORMAL_MODE)
			communicate_send_is_success(&signal, SIG_PARA_NORMAL_FRAME);
		else
			communicate_send_is_success(&signal, SIG_PARA_CASCADE_FRAME);
		free(signal.serial_standard);
	}
	pthread_mutex_unlock(&lock_tmp);
	close(fd);
}
void* serial_config_api(void* arg){
	pthread_t thread_serial[32];
	pthread_t serial_communicate_pthread;
	pthread_t serial_communicate_read_pthread;
	pthread_t serial_communicate_send_ip_pthread;
	char *serial_standard = (char*)malloc(sizeof(char)*64);
	serial_config *serial_config_a = (serial_config *)malloc(sizeof(serial_config));
	memset(serial_standard, 0, 64);
	memset(serial_config_a->serial_port, 0, sizeof(serial_config_a->serial_port));

	for(int i=0; i<32; i++){
		GET_SERIAL_NUM_CHAR(); // tmp[3] saves the contents
		strcpy(serial_config_a->serial_port[i].device_name, SERIAL_DEVICE_NAME); 
		strcpy(serial_config_a->serial_port[i].watch_serial_name, WATCH_SERIAL_NAME); 
		strcat(serial_config_a->serial_port[i].device_name, tmp);	
		strcat(serial_config_a->serial_port[i].watch_serial_name, tmp); 
		serial_config_a->serial_port[i].up_power = 1;
		serial_config_a->serial_port[i].fd = open(serial_config_a->serial_port[i].device_name, O_RDWR | O_NOCTTY | O_SYNC);
		JUDGE_ERROR_RETURN_NULL(serial_config_a->serial_port[i].fd, "Serial_config_api:open fd error. num 7");
		serial_config_a->serial_port[i].serial_standard = serial_standard;
		serial_config_a->serial_port[i].seqence = i;
	}

	int fd_communicate = open(COMMUNICATE_SERIAL_NAME, O_RDWR|O_SYNC|O_NOCTTY);
    struct termios tty_communicate;
    tcgetattr(fd_communicate, &tty_communicate);
    cfsetospeed(&tty_communicate, B9600); // Set baud rate to 9600
    cfsetispeed(&tty_communicate, B9600); // Set baud rate to 9600
    tty_communicate.c_cflag |= CS8; // 8-bit data
    tty_communicate.c_cflag &= ~PARENB; // No parity
    tty_communicate.c_cflag &= ~CSTOPB; // 1 stop bit
	tty_communicate.c_lflag &= ~ECHO; // Turn off echo
	tty_communicate.c_lflag &= ~ICANON;
    int serial_config_error = tcsetattr(fd_communicate, TCSANOW, &tty_communicate);
	JUDGE_ERROR_RETURN_NULL(serial_config_error, "Serial_config: tcsetattr error. num 10");
	close(fd_communicate);
	/* get the mode of device, whether connects to another device */
	device_mode = get_device_mode();
	if(device_mode == 0)
		left_key_process_func(NULL);
	else if (device_mode == 1)
		right_key_process_func(NULL);

	pthread_create(&serial_communicate_read_pthread, NULL, communicate_read_data, NULL);
	usleep(500);
	pthread_create(&serial_communicate_pthread, NULL, communicate_send_seiral_standard, serial_standard);
	pthread_create(&serial_communicate_send_ip_pthread, NULL, communicate_ip_addr_send, NULL);
	usleep(100);

	for(int i=0; i<16; i++){
		pthread_create(&thread_serial[i], NULL, thread_func, &serial_config_a->serial_port[i]);
	}
	JUDGE_THE_INIT_PRO();	
	pthread_cond_signal(&cond_main);
	pthread_join(serial_communicate_send_ip_pthread, NULL); // wait for end, stop main function
	for(int i=0; i<32; i++){
		close(serial_config_a->serial_port[i].fd_w);
		close(serial_config_a->serial_port[i].fd);
	}
	free(serial_standard);
	free(serial_config_a);
	return NULL;
}
