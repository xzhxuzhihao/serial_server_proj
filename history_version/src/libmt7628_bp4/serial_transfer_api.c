#include "error.h"
#include "serial_transfer_api.h"
extern wait_time transfer_wait_time;
serial_thread_signal serial_thread_signal_act;

void *handle_connection(void* arg) {
	serial_transfer * socket_serial_transfer = (serial_transfer *)arg;
	int *client_fd_ = socket_serial_transfer->client_fd;
	char port[5];
	int *serial_fd = (int *)malloc(sizeof(int));
	char reg_dir[15] = SERIAL_DEVICE_NAME;
    pthread_t read_thread;

	sprintf(port, "%d", socket_serial_transfer->serial_port);
	strcat(reg_dir, port);
	*serial_fd = open(reg_dir, SERIAL_DIRECTION);
	JUDGE_ERROR_RETURN(*serial_fd, "Serial_transfer: Open serial_fd error. num 22");
	fd_serial_client *fd_serial_client_t = (fd_serial_client *)malloc(sizeof(fd_serial_client));
	fd_serial_client_t->f_serial_fd = *serial_fd;
	fd_serial_client_t->f_client_fd = *client_fd_;
	printf("client_fd = %d\n", fd_serial_client_t->f_client_fd);
    pthread_create(&read_thread, NULL, read_data_from_serial, fd_serial_client_t);
	
	write_data_to_serial(client_fd_, serial_fd);

	pthread_cancel(read_thread);
	usleep(1000);
	PRINTF_SERIAL_COLSED();
    close(*client_fd_);
	close(*serial_fd);
	free(fd_serial_client_t);
	free(serial_fd);
	free(client_fd_);

    return NULL;
}
void *serial_thread_close(void *arg){
	serial_thread_signal *serial_thread_signal_act = (serial_thread_signal *)arg;
	usleep(5000);
	if(!serial_thread_signal_act->close_signal){
		pthread_cancel(*serial_thread_signal_act->thread);
		usleep(1000);
	}
}
char *communicate_client(int *client_fd){
	char *rec_message = (char *)malloc(sizeof(char)*10);
	memset(rec_message, 0, 10);
	SEND_MESSAGE_CLIENT(MESSAGE_0, *client_fd);
	while(1){
		unsigned char rec_nums = read(*client_fd, rec_message, 2);
		rec_nums = check_string_end(rec_message, 10);
		if(rec_nums < 0){
			EXIT_SS_RETURN_NULL("Serial_transfer: Communicate error. num 23");
		}else if(rec_nums > 0 && rec_nums < 3){
			rec_message[rec_nums] = '\0';
			break;
		}else if(rec_nums >= 3){
			EXIT_SS_RETURN_NULL("Serial_transfer: Input Error. num 24");
		}
	}
	SEND_MESSAGE_CLIENT(MESSAGE_1, *client_fd);
	return rec_message;
}
unsigned char check_string_end(char *check_str, unsigned char char_num){
	for(unsigned char i=0; i<char_num; i++){
		char check_pos = check_str[char_num-i-1];
		if(check_pos != '\0' && check_pos != '\n' && check_pos != '\r'){
			return char_num-i;
		}
	}
	return 0;
}
void* read_data_from_serial(void *s_c_fd){
	fd_serial_client *s_c_fd_ = (fd_serial_client *)s_c_fd;	
	while(1){
		char buffer[BUFFER_NUM_SED] = {0};
		int rec_nums = read(s_c_fd_->f_serial_fd, buffer, sizeof(buffer)-1);
		for(int i=0; i<5; i++)
			printf("buffer = %d\n", buffer[i]);
		printf("rec_num = %d\n", rec_nums);
		if(rec_nums < 0){
			serial_thread_signal_act.close_signal = 1;
			SEND_MESSAGE_CLIENT("Serial error.\n", s_c_fd_->f_client_fd);
			EXIT_SS_RETURN_NULL("Serial_transfer: read_data_from_serial error. num 25");
		}else if(rec_nums > 0){
		printf("f_client_fd = %d\n", s_c_fd_->f_client_fd);
			int send_nums = write(s_c_fd_->f_client_fd, buffer, rec_nums);
			if(send_nums < 0){
				serial_thread_signal_act.close_signal = 1;
				EXIT_SS_RETURN_NULL("Serial_transfer: Write error. num 26");
			}else if(send_nums == 0){
				serial_thread_signal_act.close_signal = 1;
				return NULL;
			}  
		printf("send_num = %d\n", send_nums);
		}else if(rec_nums == 0){
			serial_thread_signal_act.close_signal = 1;
			return NULL;
		}       
	}
}
void* write_data_to_serial(int *client_fd, int *serial_fd){
	while(1){
		char buffer[BUFFER_NUM_REC] = {0};
		int rec_nums = read(*client_fd, buffer, sizeof(buffer)-1);
		for(int i=0; i<5; i++)
			printf("w_buffer = %d\n", buffer[i]);
		printf("w_rec_num = %d\n", rec_nums);
		if(rec_nums < 0){
			EXIT_SS_RETURN_NULL("Serial_transfer: read_data_from_client error. num 28");
		}else if(rec_nums > 0){
			int send_nums = write(*serial_fd, buffer, rec_nums);
			if(send_nums < 0){
				SEND_MESSAGE_CLIENT("Serial error.\n", *client_fd);
				EXIT_SS_RETURN_NULL("Serial_transfer: Write error. num 27");
			}  
			//usleep((rec_nums/transfer_wait_time.data_bit*transfer_wait_time.wait_bit_per+rec_nums)*transfer_wait_time.baud_rate_value);
			usleep(5000);
		printf("w_send_num = %d\n", rec_nums);
		}else if(rec_nums == 0){
			return NULL;
		}
	}
}
int char_to_dec(char *char_array, int num){
	int sum = 0;
	int tmp = 1;
	if(num <= 0){
		return -1;
	}
	for(int i=0; i<num; i++){
		sum += (char_array[num-1-i] - '0')*tmp;
		tmp *= 10;
	}
	return sum;
}
