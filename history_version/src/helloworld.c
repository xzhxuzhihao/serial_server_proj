#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <termios.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 36) )

void handle_config_change(const char* filename) {
    // Here, put your code that reads the new configuration values from the file
    // and applies them to the serial port.
	printf("OK, has modify\n");
}

int main() {
    struct termios tty;
	int fd = open("/dev/ttyACM0", O_RDWR);
    tcgetattr(fd, &tty);
    cfsetospeed(&tty, B921600); // Set baud rate to 9600
    tty.c_cflag |= CS8; // 8-bit data
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
	tty.c_lflag &= ~ECHO; // Turn off echo
    int error_judge = tcsetattr(fd, TCSANOW, &tty);
	
}
