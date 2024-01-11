#include <stdio.h>
#include <sys/inotify.h>
#include <errno.h>
#include <string.h> // for strerror()

int main() {
    int fd = inotify_init();
    int fd1= inotify_init();
    int fd2= inotify_init();
    int fd3= inotify_init();
    int fd4= inotify_init();
    int fd5= inotify_init();
    int fd6= inotify_init();
    int fd7= inotify_init();
    int fd8= inotify_init();
    int fd9= inotify_init();
    int fd10= inotify_init();
    int fd11 = inotify_init();
    int fd12 = inotify_init();
    int fd13 = inotify_init();
    int fd14 = inotify_init();
    int fd15 = inotify_init();
    if (fd < 0) {
        perror("inotify_init failed");
        return 1;
    }

    int wd = inotify_add_watch(fd, "/etc/config/test_serial_para_0", IN_MODIFY);
    int wd1= inotify_add_watch(fd1, "/etc/config/test_serial_para_1", IN_MODIFY);
    int wd2= inotify_add_watch(fd2, "/etc/config/test_serial_para_2", IN_MODIFY);
    int wd3= inotify_add_watch(fd3, "/etc/config/test_serial_para_3", IN_MODIFY);
    int wd4= inotify_add_watch(fd4, "/etc/config/test_serial_para_4", IN_MODIFY);
    int wd5= inotify_add_watch(fd5, "/etc/config/test_serial_para_5", IN_MODIFY);
    int wd6= inotify_add_watch(fd6, "/etc/config/test_serial_para_6", IN_MODIFY);
    int wd7= inotify_add_watch(fd7, "/etc/config/test_serial_para_7", IN_MODIFY);
    int wd8= inotify_add_watch(fd8, "/etc/config/test_serial_para_8", IN_MODIFY);
    int wd9= inotify_add_watch(fd9, "/etc/config/test_serial_para_9", IN_MODIFY);
    int wd10 = inotify_add_watch(fd10, "/etc/config/test_serial_para_10", IN_MODIFY);
    int wd11 = inotify_add_watch(fd11, "/etc/config/test_serial_para_11", IN_MODIFY);
    int wd12 = inotify_add_watch(fd12, "/etc/config/test_serial_para_12", IN_MODIFY);
    int wd13 = inotify_add_watch(fd13, "/etc/config/test_serial_para_13", IN_MODIFY);
    int wd14 = inotify_add_watch(fd14, "/etc/config/test_serial_para_14", IN_MODIFY);
    int wd15 = inotify_add_watch(fd15, "/etc/config/test_serial_para_15", IN_MODIFY);
    if (wd < 0 || wd1 < 0|| wd2 < 0|| wd3 < 0|| wd4 < 0|| wd5 < 0|| wd6 < 0|| wd7 < 0|| wd8 < 0|| wd9 < 0|| wd10 < 0|| wd11 < 0|| wd12 < 0|| wd13 < 0|| wd14 < 0|| wd15 < 0) {
        perror("inotify_add_watch failed");
        printf("Error message: %s\n", strerror(errno));
        printf("Error message:\n");
        return 1;
    }

        printf("ok\n");

    // rest of your code...

    return 0;
}

