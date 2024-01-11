#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>

void *thread_function_0(void *arg){
	char buf_name[10] = {0};
	int fd = inotify_init();
	int wd = inotify_add_watch(fd, "/tmp/gdb_test", IN_MODIFY);

	printf("test: gdb:function wait\n");
	int fd_read = read(fd, buf_name, 10);
	if(fd_read > 0){
		printf("ok, have modify.\n");
	}
}
