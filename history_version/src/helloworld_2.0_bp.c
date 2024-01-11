#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <termios.h>


const char *REG_DIR = "/dev/";
char REG[15];
int32_t fd;

void write_reg(char *str, char *reg, u_int16_t write_num);
u_int16_t get_data(char *str, u_int16_t read_num);
u_int16_t detect(char *dir);
void add_to_degdir(char *reg_dir, char *add);

int main(int argu_num, char *argu_data[]){
	char reg_dir[10];
	char dir[15] = "/tmp/uart_data";

	add_to_degdir(reg_dir, argu_data[1]);
	while(1){
		if(detect(dir) > 10){
			fd = open(dir, O_RDONLY);
			break;
		}
	}

	char *pre = (char *)malloc(55);
	memset(pre, 0, 55);
	char *new_;
	u_int16_t num_tmp = 55;
	char tmp_data[55];
	bool poi = 0;

	while(1){
		if(!poi)
			usleep(10000);
		u_int16_t num = get_data(tmp_data, 50);
		if(num){
			if(num == 50){
				num_tmp += 50;
				new_ = (char *)malloc(num_tmp);
				memset(new_, 0, num_tmp);
				strcpy(new_, pre);
				strcat(new_, tmp_data);
				new_[num_tmp-55] = '\0';
				free(pre);
				pre = new_;
				poi = 1;
			}
			if(num < 50){
				strcat(pre, tmp_data);
				write_reg(pre, reg_dir, strlen(pre));
				free(pre);
				pre = (char *)malloc(55);
				memset(pre, 0, 55);	
				num_tmp = 55;
				poi = 1;
			}
		}
		if(!num && poi == 1){
			if(strlen(pre)){
				write_reg(pre, reg_dir, strlen(pre));
			}
			free(pre);
			pre = (char *)malloc(55);
			memset(pre, 0, 55);
			num_tmp = 55;
			poi = 0;
		}
	}

	return 0;
}

void add_to_degdir(char *reg_dir, char *add){
	memset(reg_dir, 0, 10);
	if(add[0] == 'c'){
		strcpy(reg_dir, "ttyS0");
		return;
	}
	strcpy(reg_dir, "ttyACM");
	strcat(reg_dir, add);
}

unsigned char *str_to_num(char *wrt_str){
	u_int32_t len = strlen(wrt_str)/2;
	unsigned char *res_num = (unsigned char *)malloc(len);
	for(u_int32_t i=0; i<len; i++){
		unsigned char sum = 0;
		sum += (wrt_str[2*i] > 'a') ? (wrt_str[2*i]-'a'+10)*16:(wrt_str[2*i]-'0')*16;
		sum += (wrt_str[2*i+1] > 'a') ? wrt_str[2*i+1]-'a'+10:wrt_str[2*i+1]-'0';
		res_num[i] = sum;
	}
	return res_num;
}
void write_reg(char *str, char *reg, u_int16_t write_num){
	strcpy(REG, REG_DIR);
	strcat(REG, reg);
	int32_t fd = open(REG, O_WRONLY | O_NOCTTY | O_SYNC);
	unsigned char *res_num;
    
    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetospeed(&tty, B921600); // Set baud rate to 9600
    tty.c_cflag |= CS8; // 8-bit data
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tcsetattr(fd, TCSANOW, &tty);
    
	res_num = str_to_num(str);
    write(fd, res_num, write_num/2);
	printf("Recieved num = %d\n", write_num);

    close(fd);
}
u_int16_t get_data(char *str, u_int16_t read_num){
	u_int16_t act_read = 0;
	act_read = read(fd, str, read_num);
	str[act_read] = '\0';

	return act_read;
}
u_int16_t detect(char *dir){
	char tmp_data[10];
	int32_t fd = open(dir, O_RDONLY);	
	u_int16_t num = 10;
	u_int16_t sum = 0;
	while(num == 10){
		num = read(fd, tmp_data, 10);
		sum += num;
	}
	close(fd);

	return sum+10;
}
