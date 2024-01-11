#include "error.h"
#include "serial_transfer_api.h"

void *handle_connection(serial_transfer *socket_serial_transfer) {
	int *client_fd_ = (int *)socket_serial_transfer->client_fd;
	char port[5];
	int *serial_fd = (int *)malloc(sizeof(int));
	char reg_dir[15] = SERIAL_DEVICE_NAME;
    pthread_t read_thread;

	//port = communicate_client(client_fd_); 
	//CONNECT_ERROR_JUDGE_NULL(port, "Serial_transfer: Communicate_client error. num 21", *client_fd_);
	sprintf(port, "%d", socket_serial_transfer->serial_port);
	strcat(reg_dir, port);
	*serial_fd = open(reg_dir, SERIAL_DIRECTION);
	CONNECT_ERROR_JUDGE_NEGATIVE(*serial_fd, "Serial_transfer: Open serial_fd error. num 22", *client_fd_);
	fd_serial_client *fd_serial_client_t = (fd_serial_client *)malloc(sizeof(fd_serial_client));
	fd_serial_client_t->f_serial_fd = *serial_fd;
	fd_serial_client_t->f_client_fd = *client_fd_;
    pthread_create(&read_thread, NULL, read_data_from_serial, fd_serial_client_t);

	write_data_to_serial(client_fd_, serial_fd);

    close(*client_fd_);
	close(*serial_fd);
	free(fd_serial_client_t);
	free(serial_fd);
	free(client_fd_);
	free(port);

    return NULL;
}
char *communicate_client(int *client_fd){
	char *rec_message = (char *)malloc(sizeof(char)*10);
	memset(rec_message, 0, 10);
	SEND_MESSAGE_CLIENT(MESSAGE_0);
	while(1){
		unsigned char rec_nums = read(*client_fd, rec_message, 2);
		rec_nums = check_string_end(rec_message, 10);
		if(rec_nums < 0){
			CONNECT_ERROR_JUDGE_NEGATIVE(rec_nums, "Serial_transfer: Communicate error. num 23", *client_fd);
		}else if(rec_nums > 0 && rec_nums < 3){
			rec_message[rec_nums] = '\0';
			break;
		}else if(rec_nums >= 3){
			EXIT_SS("Serial_transfer: Input Error. num 24");
			return NULL;
		}
	}
	SEND_MESSAGE_CLIENT(MESSAGE_1);
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
		if(rec_nums < 0){
			CONNECT_ERROR_JUDGE_NEGATIVE(rec_nums, "Serial_transfer: read_data_from_serial error. num 25", s_c_fd_->f_client_fd);
		}else if(rec_nums > 0){
			int send_nums = write(s_c_fd_->f_client_fd, buffer, rec_nums);
			if(send_nums < 0){
				EXIT_SS("Serial_transfer: Write error. num 26");
				exit(1);
			}  
		}else if(rec_nums == 0){
			EXIT_SS("Serial_transfer: Connect closed.");
			return NULL;
		}       
	}
}
void* write_data_to_serial(int *client_fd, int *serial_fd){
	while(1){
		char buffer[BUFFER_NUM_REC] = {0};
		int rec_nums = read(*client_fd, buffer, sizeof(buffer)-1);
		if(rec_nums < 0){
			CONNECT_ERROR_JUDGE_NEGATIVE(rec_nums, "Serial_transfer: read_data_from_client error. num 28", *client_fd);
		}else if(rec_nums > 0){
			int send_nums = write(*serial_fd, buffer, rec_nums);
			if(send_nums < 0){
				EXIT_SS("Serial_transfer: Write error. num 27");
				exit(1);
			}  
		}else if(rec_nums == 0){
			EXIT_SS("Serial_tranfer: Connect closed.");
			return NULL;
		}
	}
}
