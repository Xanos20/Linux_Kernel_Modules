

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string.h>
#include <sys/time.h>
#include <linux/ioctl.h>

#include <stdint.h>

#include <unistd.h>
#include <sys/ioctl.h>


int fd_plat1;
int fd_plat2;


void open_fds() {
	fd_plat1 = open("/dev/hcsr_1", O_RDWR);
	if(fd_plat1 < 0) {
		fprintf(stderr, "Cannot open fd_plat1 code = %d\n", fd_plat1);
		exit(-1);
	}
	fd_plat2 = open("/dev/hcsr_2", O_RDWR);
	if(fd_plat2 < 0) {
		fprintf(stderr, "Cannot open fd_plat2 code = %d\n", fd_plat2);
		exit(-1);
	}
	return;

}

void close_fds() {
	int ret = 0;
	ret = close(fd_plat1);
	if(ret != 0) {
		fprintf(stderr, "Cannont close fd_plat1 code = %d\n", ret);
		exit(-1);
	}
	ret = close(fd_plat2);
	if(ret != 0) {
		fprintf(stderr, "Cannont close fd_plat2 code = %d\n", ret);
		exit(-1);
	}
	return;
}

int main() {
	open_fds();
	close_fds();
	return 0;
}
