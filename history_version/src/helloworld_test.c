#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define IS_ERROR(x) if(x == 1) ; else return -1; 
#define Write_Data(x) strcpy(write_data, "0x0000");\
						for(int i=0; i<4; i++)\
							write_data[6+i] = read_data[(x)+i];\
						write_data[10] = '\0';
												
#define Write_Data_mode4(x) strcpy(write_data, "0x");\
							write_data[2] = '0';\
							write_data[3] = (x > 8) ? 'a'+x-9 : '0'+x+1;\
							write_data[4] = read_data[14];\
							write_data[5] = read_data[15];\
							write_data[6] = '0';\
							write_data[7] = 'f';\
							write_data[8] = read_data[0];\
							write_data[9] = read_data[1];\
							write_data[10] = '\0';
#define Write_Data_mode3(x,y) strcpy(write_data, "0x");\
						 for(int i=0; i<8; i++){\
						 if(i < 4){\
							    write_data[2+i] = read_data[(x)+i];\
							}else{\
								write_data[2+i] = read_data[i-(y)];\
							}\
						  }\
						  write_data[10] = '\0';
bool start_flag;
int fd;
struct data_info {
	int num;
	int mode;
};
struct channel_data {
	char *channel0;
	char *channel1;
	char *channel2;
	char *channel3;
};

const char *REG_DIR = "/sys/kernel/debug/10002000.pcm/";
char REG[50];

bool write_reg(char *str, char *reg){
	int32_t flag; 
	strcat(REG, REG_DIR);
	strcat(REG, reg);
	int32_t fd = open(REG, O_WRONLY);
	flag = write(fd, str, 10);
	close(fd);
	memset(REG, 0, 50);
	
	return flag;
}
bool read_reg(char *buf, char *reg){
	int32_t flag;
	strcat(REG, REG_DIR);
	strcat(REG, reg);
	int32_t fd = open(REG, O_RDONLY); 
	flag = read(fd, buf, 10);
	buf[10] = '\0';
	printf("27 114chb1ff= %s ok\n", buf);
	close(fd);
	memset(REG, 0, 50);

	return flag;
}
void stop(){
	write_reg("0x00440000", "000_glb_cfg");
}
int bit4_exp(int selt, char *reg){
	char buf[14];
	if(read_reg(buf, reg))
		return buf[9-selt] >= 'a' ? buf[9-selt]-'a'+10:buf[9-selt]-'0';
	else 
		return -1;
}
void send_config(void){
	write_reg("0x0000ff00", "080_ch0_fifo");
	write_reg("0x00000000", "084_ch1_fifo");
	write_reg("0x0000ffff", "088_ch2_fifo");
	write_reg("0x00000000", "08c_ch3_fifo");
}
void write_reg_fst1byte(char *write_data,char *read_data, int num){
	strcpy(write_data, "0x0000");
	write_data[6] = '0';
	if(num >= 2)
		write_data[7] = 'a'+num-2;
	else 
		write_data[7] = '0'+num+8;
	write_data[8] = read_data[0];
	write_data[9] = read_data[1];
	write_data[10] = '\0';
}
int fork_proc(char *read_data, int *poi){
	int filedes[2];
	int num;
	pid_t childpid;
	char write_buf[] = "aa";
	char read_buf[10];
			printf("46 poi = %d\n", *poi);
	if(*poi){
		pipe(filedes);
		if((childpid = fork()) == 0){	
			close(filedes[1]);
			while(!(num=read(filedes[0], read_buf, 1))){
				if(bit4_exp(0, "114_chb1_ff_status") >= 4){
					write_reg("0x00000000", "080_ch0_fifo");
					write_reg("0x00000000", "084_ch1_fifo");
					write_reg("0x00000000", "088_ch2_fifo");
					write_reg("0x00000000", "08c_ch3_fifo");
				}
			}
			exit(0);
		}else{
			close(filedes[0]);
			//num = read(fd, read_data, 28);
			printf("read_data = %s \n", read_data);
			write(filedes[1], write_buf, sizeof(write_buf));
		}
		
			printf("fork is running\n");
	}else{
	//	num = read(fd, read_data, 28);
		;
	}
	return num;
}
bool get_data_struct(char *read_data, struct data_info* data_struct, int *poi){
	if(start_flag){
		fd = open("/tmp/pcm_data", O_RDONLY);
		usleep(1000);
		close(fd);
		usleep(1000);
		fd = open("/tmp/pcm_data", O_RDONLY);
		start_flag = false;
	}
//	int num = fork_proc(read_data, poi); 
	int num = read(fd, read_data, 28);
	read_data[num] = '\0';
//	//printf("26 get_data_num = %d \n", num);
//	//printf("28 read_data = %s \n", read_data);/
	printf("read_data = %s \n", read_data);
	
	if(num == -1){
		data_struct->num = 0;
		return false;
	}
	read_data[num] = '\0';
	num = 0;
	for(int i=0; read_data[i] != '\0' && read_data[i] != '\n'; i++)
		num++;
	if(num%2 != 0)
		num--;
	if(num == 28){
		data_struct->num = 14;
		data_struct->mode = 4;
	}else if(num < 28 && num > 14){
		data_struct->num = num/2;
		data_struct->mode = 3;
		return false;
	}else if(num == 14){
		data_struct->num = 7;
		data_struct->mode = 2;
		return false;
	}else if(num < 14 && num > 0){
		data_struct->num = num/2;
		data_struct->mode = 1;
		return false;
	}else{
		data_struct->num = 0;
		return false;
	}
	return true;
}
void write_data16_byte(struct data_info* data_struct, char *read_data, struct channel_data *channel_wrt){
	char write_data[11];
	//printf("29 read_data = %s \n", read_data);
	//printf("6ok\n");
	//printf("7 : data_struct->mode = %d\n", data_struct->mode);
	if(data_struct->mode == 0){
	//printf("5ok\n");
		strcpy(channel_wrt->channel0, "0x00008c11");
		strcpy(channel_wrt->channel1, "0x00000000");
		strcpy(channel_wrt->channel2, "0x00000000");
		strcpy(channel_wrt->channel3, "0x00000000");
	//printf("6ok\n");
	}
	//printf("7ok\n");
	if(data_struct->mode == 1){
		strcpy(write_data, "0x");
		if(data_struct->num == 1){
			write_reg_fst1byte(write_data, read_data, 1);
			strcpy(channel_wrt->channel0, write_data);
			//printf("77 wirte_data0 = %s\n", write_data);
			strcpy(channel_wrt->channel1, "0x00000000");
			//printf("76 wirte_data1 = %s\n", channel_wrt->channel1);
			strcpy(channel_wrt->channel2, "0x00000000");
			//printf("75 wirte_data2 = %s\n", channel_wrt->channel2);
			strcpy(channel_wrt->channel3, "0x00000000");
			//printf("74 wirte_data3 = %s\n", channel_wrt->channel3);
		}
		if(data_struct->num == 2){
			write_reg_fst1byte(write_data, read_data, 2);
			strcpy(channel_wrt->channel0, write_data);
			//printf("73 wirte_data0 = %s\n", write_data);
				
			strcpy(write_data, "0x0000");
			for(int i=0; i<4; i++){
				if(i<2)
					write_data[6+i] = read_data[2+i];
				else 
					write_data[6+i] = '0'; 
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel1, write_data);
			//printf("72 wirte_data1 = %s\n", write_data);
			strcpy(channel_wrt->channel2, "0x00000000");
			//printf("71 wirte_data2 = %s\n", channel_wrt->channel2);
			strcpy(channel_wrt->channel3, "0x00000000");
			//printf("70 wirte_data3 = %s\n", channel_wrt->channel3);
		}
		if(data_struct->num == 3){
			write_reg_fst1byte(write_data, read_data, 3);
			strcpy(channel_wrt->channel0, write_data);
			//printf("69 wirte_data0 = %s\n", write_data);
			
			strcpy(write_data, "0x0000");
			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[2+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel1, write_data);
			//printf("68 wirte_data1 = %s\n", write_data);
			strcpy(channel_wrt->channel2, "0x00000000");
			//printf("67 wirte_data2 = %s\n", channel_wrt->channel2);
			strcpy(channel_wrt->channel3, "0x00000000");
			//printf("66 wirte_data3 = %s\n", channel_wrt->channel3);
		}
		if(data_struct->num == 4){
			write_reg_fst1byte(write_data, read_data, 4);
			strcpy(channel_wrt->channel0, write_data);
			//printf("65 wirte_data0 = %s\n", write_data);

			strcpy(write_data, "0x0000");
			for(int i =0; i<4; i++){
				write_data[6+i] = read_data[2+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel1, write_data);
			//printf("64 wirte_data1 = %s\n", write_data);
			
			strcpy(write_data, "0x0000");
			write_data[6] = read_data[6];
			write_data[7] = read_data[7];
			write_data[8] = '0';
			write_data[9] = '0';
			write_data[10] = '\0';
			strcpy(channel_wrt->channel2, write_data);
			//printf("63 wirte_data2 = %s\n", write_data);
			
			strcpy(channel_wrt->channel3, "0x00000000");
			//printf("62 wirte_data3 = %s\n", channel_wrt->channel3);
		}	
		if(data_struct->num == 5){
			write_reg_fst1byte(write_data, read_data, 5);
			strcpy(channel_wrt->channel0, write_data);
			//printf("61 wirte_data0 = %s\n", write_data);

			strcpy(write_data, "0x0000");

			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[2+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel1, write_data);
			//printf("60 wirte_data1 = %s\n", write_data);

			strcpy(write_data, "0x0000");

			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[6+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel2, write_data);
			//printf("59 wirte_data2 = %s\n", write_data);
			
			strcpy(channel_wrt->channel3, "0x00000000");
			//printf("58 wirte_data3 = %s\n", channel_wrt->channel3);
		 }
		if(data_struct->num == 6){
			write_reg_fst1byte(write_data, read_data, 6);
			strcpy(channel_wrt->channel0, write_data);
			//printf("57 wirte_data0 = %s\n", write_data);
			
			strcpy(write_data, "0x0000");

			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[2+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel1, write_data);
			//printf("56 wirte_data1 = %s\n", write_data);

			strcpy(write_data, "0x0000");

			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[6+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel2, write_data);
			//printf("55 wirte_data2 = %s\n", write_data);

			strcpy(write_data, "0x0000");
			for(int i=0; i<2; i++)
				write_data[6+i] = read_data[10+i];
			write_data[8] = '0';
			write_data[9] = '0';
			write_data[10] = '\0';
			strcpy(channel_wrt->channel3, write_data);
			//printf("54 wirte_data3 = %s\n", write_data);
		}
	}				
	if(data_struct->mode == 2){
		write_reg_fst1byte(write_data, read_data, 7);
		strcpy(channel_wrt->channel0, write_data);

		Write_Data(2);
		strcpy(channel_wrt->channel1, write_data);
			//printf("50 wirte_data1 = %s\n", write_data);
		Write_Data(6);
		strcpy(channel_wrt->channel2, write_data);
			//printf("51 wirte_data2 = %s\n", write_data);
		Write_Data(10);
		strcpy(channel_wrt->channel3, write_data);
			//printf("52 wirte_data3 = %s\n", write_data);

	}
	if(data_struct->mode == 3){
	//printf("30 read_data = %s \n", read_data);
			//printf("34 wirte_data0 = %s\n", write_data);
		if(data_struct->num == 8){
			Write_Data_mode4(8);
	//printf("31 write_data0 = %s \n", write_data);
		strcpy(channel_wrt->channel0, write_data);
			Write_Data(2);
			strcpy(channel_wrt->channel1, write_data);
			//printf("35 wirte_data1 = %s\n", write_data);
			Write_Data(6);
			strcpy(channel_wrt->channel2, write_data);
			//printf("36 wirte_data2 = %s\n", write_data);
			Write_Data(10);
			strcpy(channel_wrt->channel3, write_data);
			//printf("37 wirte_data3 = %s\n", write_data);
		}else if(data_struct->num == 9){	
			Write_Data_mode4(9);
	//printf("31 write_data0 = %s \n", write_data);
		strcpy(channel_wrt->channel0, write_data);
			strcpy(write_data, "0x");
			write_data[2] = read_data[16];
			write_data[3] = read_data[17];
			write_data[4] = '0';
			write_data[5] = '0';
			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[2+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel1, write_data);
			//printf("38 wirte_data1 = %s\n", write_data);
			Write_Data(6);
			strcpy(channel_wrt->channel2, write_data);
			//printf("39 wirte_data2 = %s\n", write_data);
			Write_Data(10);
			strcpy(channel_wrt->channel3, write_data);
			//printf("40 wirte_data3 = %s\n", write_data);
		}else if(data_struct->num == 10){
			Write_Data_mode4(10);
	//printf("31 write_data0 = %s \n", write_data);
		strcpy(channel_wrt->channel0, write_data);
			Write_Data_mode3(16, 2);
			//printf("34 print the asci of bit 19 20 21 22 : %d %d %d %d\n", read_data[18], read_data[19], read_data[20], read_data[21]);
			strcpy(channel_wrt->channel1, write_data);
			//printf("wirte_data1 = %s\n", write_data);
			Write_Data(6);
			strcpy(channel_wrt->channel2, write_data);
			//printf("wirte_data2 = %s\n", write_data);
			Write_Data(10);
			strcpy(channel_wrt->channel3, write_data);
			//printf("wirte_data3 = %s\n", write_data);
		}else if(data_struct->num == 11){
			Write_Data_mode4(11);
	//printf("31 write_data0 = %s \n", write_data);
		strcpy(channel_wrt->channel0, write_data);
			Write_Data_mode3(16, 2);
			strcpy(channel_wrt->channel1, write_data);
			//printf("41 wirte_data1 = %s\n", write_data);
			strcpy(write_data, "0x");
			write_data[2] = read_data[20];
			write_data[3] = read_data[21];
			write_data[4] = '0';
			write_data[5] = '0';
			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[6+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel2, write_data);
			//printf("42 wirte_data2 = %s\n", write_data);
			Write_Data(10);
			strcpy(channel_wrt->channel3, write_data);
			//printf("43 wirte_data3 = %s\n", write_data);
		}else if(data_struct->num == 12){
			Write_Data_mode4(12);
	//printf("31 write_data0 = %s \n", write_data);
		strcpy(channel_wrt->channel0, write_data);
			Write_Data_mode3(16, 2);
			strcpy(channel_wrt->channel1, write_data);
			//printf("44 wirte_data1 = %s\n", write_data);
			Write_Data_mode3(20, -2);
			strcpy(channel_wrt->channel2, write_data);
			//printf("45 wirte_data2 = %s\n", write_data);
			Write_Data(10);
			strcpy(channel_wrt->channel3, write_data);
			//printf("46 wirte_data3 = %s\n", write_data);
		}else if(data_struct->num == 13){
			Write_Data_mode4(13);
	//printf("31 write_data0 = %s \n", write_data);
		strcpy(channel_wrt->channel0, write_data);
			Write_Data_mode3(16, 2);
			strcpy(channel_wrt->channel1, write_data);
			//printf("47 wirte_data1 = %s\n", write_data);
			Write_Data_mode3(20, -2);
			strcpy(channel_wrt->channel2, write_data);
			//printf("48 wirte_data2 = %s\n", write_data);
			strcpy(write_data, "0x");
			write_data[2] = read_data[24];
			write_data[3] = read_data[25];
			write_data[4] = '0';
			write_data[5] = '0';
			for(int i=0; i<4; i++){
				write_data[6+i] = read_data[10+i];
			}
			write_data[10] = '\0';
			strcpy(channel_wrt->channel3, write_data);
			//printf("49 wirte_data3 = %s\n", write_data);
		}
	}

	if(data_struct->mode == 4){
		Write_Data_mode4(14);
		strcpy(channel_wrt->channel0, write_data);
		printf("16 wirte_data0 = %s\n", write_data);
		//printf("11ok\n");
		strcpy(write_data, "0x");
		for(int i=0; i<8; i++){
			if(i < 4){
				write_data[2+i] = read_data[16+i];
			}else{
				write_data[2+i] = read_data[i-2];
			}
		}
		//printf("12ok\n");
		write_data[10] = '\0';
		strcpy(channel_wrt->channel1, write_data);
		printf("17 wirte_data1 = %s\n", write_data);
		
		strcpy(write_data, "0x");
		for(int i=0; i<8; i++){
			if(i < 4){
				write_data[2+i] = read_data[20+i];
			}else{
				write_data[2+i] = read_data[i+2];
			}
		}
		//printf("13ok\n");
		write_data[10] = '\0';
		strcpy(channel_wrt->channel2, write_data);
		printf("18 wirte_data2 = %s\n", write_data);
			
		strcpy(write_data, "0x");
		for(int i=0; i<8; i++){
			if(i < 4){
				write_data[2+i] = read_data[24+i];
			}else{
				write_data[2+i] = read_data[i+6];
			}
		}
		write_data[10] = '\0';
		strcpy(channel_wrt->channel3, write_data);
		printf("14 wirte_data3 = %s\n", write_data);
	}
}
bool init_reg(struct channel_data* channel_wrt, char *read_data, struct data_info* data_struct ,int *poi){
	int32_t flag;

	write_reg("0x00440000", "000_glb_cfg");
	write_reg("0x03000000", "004_pcm_cfg");
	write_reg("0x00000000", "050_divcomp_cfg");
	write_reg("0x00000000", "054_divint_cfg");
	write_reg("0x28000000", "030_fsync_cfg");
	write_reg("0x00000000", "020_cha0_cfg");
	write_reg("0x00000000", "024_chb0_cfg");
	write_reg("0x00000000", "120_cha1_cfg");
	write_reg("0x00000000", "124_chb1_cfg");
	usleep(100000);

/*	
	write_reg("0x00000004", "034_cha0_cfg2");	
	write_reg("0x00000004", "134_cha1_cfg2");	
	write_reg("0x00000004", "038_chb0_cfg2");	
	write_reg("0x00000004", "138_chb1_cfg2");	
	usleep(100000);
*/
	write_reg("0x00000000", "034_cha0_cfg2");	
	write_reg("0x00000000", "134_cha1_cfg2");	
	write_reg("0x00000000", "038_chb0_cfg2");	
	write_reg("0x00000000", "138_chb1_cfg2");	
	usleep(100000);

	write_reg("0x06000001","004_pcm_cfg");
	write_reg("0x00000000", "020_cha0_cfg");
	write_reg("0x00000010", "024_chb0_cfg");
	write_reg("0x00000020", "120_cha1_cfg");
	write_reg("0x00000030", "124_chb1_cfg");
	//printf("2ok\n");
	

	for(int i=0; i<4; i++){
		/*
		if(i == 0){
			data_struct->mode = 0;
			data_struct->num = 0;
	//printf("8ok\n");
		}else{
			data_struct->mode = 4;
			data_struct->num = 14;
	//printf("10ok\n");
		}
	//printf("9 : data_struct->mode = %d\n", data_struct->mode);
	//printf("3ok\n");
	*/
		if(i == 0){
			data_struct->mode = 0;
			data_struct->num = 0;
		}else if(!get_data_struct(read_data, data_struct, poi) && data_struct->num == 0){
			printf("45 poi = %d\n", *poi);
			strcpy(channel_wrt->channel0, "0x00000000"); //test
			strcpy(channel_wrt->channel1, "0x00000000");
			strcpy(channel_wrt->channel2, "0x00000000");
			strcpy(channel_wrt->channel3, "0x00000000");
			write_reg(channel_wrt->channel0, "080_ch0_fifo");
			write_reg(channel_wrt->channel1, "084_ch1_fifo");
			write_reg(channel_wrt->channel2, "088_ch2_fifo");
			write_reg(channel_wrt->channel3, "08c_ch3_fifo");
			continue;
		}
					
	//printf("19 : read_data = %s\n", read_data);
	//printf("4ok\n");
		write_data16_byte(data_struct, read_data, channel_wrt);
		write_reg(channel_wrt->channel0, "080_ch0_fifo");
	printf("20 : channel_wrt->channel0 = %s\n", channel_wrt->channel0);
		write_reg(channel_wrt->channel1, "084_ch1_fifo");
	printf("okokok");
	printf("21 : channel_wrt->channel1 = %s\n", channel_wrt->channel1);
		write_reg(channel_wrt->channel2, "088_ch2_fifo");
	printf("22 : channel_wrt->channel2 = %s\n", channel_wrt->channel2);
		write_reg(channel_wrt->channel3, "08c_ch3_fifo");
	printf("23 : channel_wrt->channel3 = %s\n", channel_wrt->channel3);
	}

	write_reg("0x8044000f", "000_glb_cfg");
	write_reg("0x80000040", "050_divcomp_cfg");
	write_reg("0x000003fc", "054_divint_cfg");
	write_reg("0xb8000001", "030_fsync_cfg");

	write_reg("0x46000001","004_pcm_cfg");
	/*
	printf("25 out_start 114chb1ff = %d\n", bit4_exp(0, "114_chb1_ff_status"));
	write_reg("0x00000004", "138_chb1_cfg2");	
	printf("25 out_start 114chb1ff = %d\n", bit4_exp(0, "114_chb1_ff_status"));
	if(bit4_exp(0, "114_chb1_ff_status") != 8)
		write_reg("0x00000000", "138_chb1_cfg2");	

	printf("25 out_start 114chb1ff = %d\n", bit4_exp(0, "114_chb1_ff_status"));
	*/
	return true;
}
void send_0byte_wait(struct channel_data *channel_wrt){
//	strcpy(channel_wrt->channel0, "0x08000800");
	strcpy(channel_wrt->channel0, "0x00000000"); //test
	strcpy(channel_wrt->channel1, "0x00000000");
	strcpy(channel_wrt->channel2, "0x00000000");
	strcpy(channel_wrt->channel3, "0x00000000");
	write_reg(channel_wrt->channel0, "080_ch0_fifo");
	write_reg(channel_wrt->channel1, "084_ch1_fifo");
	write_reg(channel_wrt->channel2, "088_ch2_fifo");
	write_reg(channel_wrt->channel3, "08c_ch3_fifo");
}
void send_0byte_end(struct channel_data *channel_wrt){
	close(fd);
	while(1){
		if(bit4_exp(0, "114_chb1_ff_status") >= 4){
			send_0byte_wait(channel_wrt);
		}
	}
}	

bool judge_byte(){
	char test_buf[32];
	int fd = open("/tmp/pcm_data", O_RDONLY);
	int32_t num = read(fd, test_buf, 28);
	
	if(num == -1){
		return false;
	}
	test_buf[num] = '\0';
	num = 0;
	for(int i=0; test_buf[i] != '\0' && test_buf[i] != '\n'; i++)
		num++;
	close(fd);
	usleep(1000);
	return num/2 > 0 ? true : false;

}
int main(){
	char read_data[32];
	struct data_info *data_struct = (struct data_info*)malloc(sizeof(struct data_info));
	int32_t num = 0;
	bool flag = false;
	time_t * t1, *t2;
	struct channel_data *channel_wrt = (struct channel_data*)malloc(sizeof(struct channel_data));
	int *poi = (int*)malloc(sizeof(int));
	*poi = 0;

	channel_wrt->channel0 = (char*)malloc(sizeof(char)*12);
	channel_wrt->channel1 = (char*)malloc(sizeof(char)*12);
	channel_wrt->channel2 = (char*)malloc(sizeof(char)*12);
	channel_wrt->channel3 = (char*)malloc(sizeof(char)*12);
	//printf("1ok\n");
	start_flag = true;
		printf("hello\n");
		printf("433 poi = %d\n", *poi);
	while(1){	
		if(judge_byte()){
			init_reg(channel_wrt, read_data, data_struct, poi);
			break;
		}
	}    //test
	num += 42;	
	t1 = (time_t*)malloc(sizeof(time_t));
	t2 = (time_t*)malloc(sizeof(time_t));
	time(t1);
	while(1){
	//printf("25 out_start 114chb1ff = %d\n", bit4_exp(0, "114_chb1_ff_status"));
	// for(int i=0; i<20; i++){
		if(bit4_exp(0, "114_chb1_ff_status") >= 4){
			for(int i=0; i<4; i++){
				get_data_struct(read_data, data_struct, poi); 
				if(data_struct->num == 0){
					time(t2);
					//printf("all send %d \n", num);
					/*
					if(*t2 - *t1 >= 20){
						close(fd);
						//printf("close");
					}
					*/
					int i=0;
					while(i<25){
						if(bit4_exp(0, "114_chb1_ff_status") >= 4){
							send_0byte_wait(channel_wrt);
							i += 1;
						}
					}
				}else{
					time(t1);
					time(t2);
					num += data_struct->num;
					write_data16_byte(data_struct, read_data, channel_wrt);
					write_reg(channel_wrt->channel0, "080_ch0_fifo");
					write_reg(channel_wrt->channel1, "084_ch1_fifo");
					write_reg(channel_wrt->channel2, "088_ch2_fifo");
					write_reg(channel_wrt->channel3, "08c_ch3_fifo");
					printf("40 write");
				}
			}
	//printf("26 in_ 114chb1ff = %d\n", bit4_exp(0, "114_chb1_ff_status"));
		}
	}
	 close(fd);
	 return 0;
	//printf("24 out_114chb1ff = %d\n", bit4_exp(0, "114_chb1_ff_status"));

	the_end_deal:
		send_0byte_wait(channel_wrt);
	//stop();
	return 0;
}
