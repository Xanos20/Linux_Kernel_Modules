/*   A test program for /dev/kbuf
		To run the program, enter "kbuf_tester show" to show the current contend of the buffer.
				enter "kbuf_tester write <input_string> to append the <input_string> into the buffer

*/

// TODO: define macros in main

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
/*#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/rtc.h>*/
#include <time.h>


pthread_t p1;
int fd;


void* access_write() {
	char pbuff[1024];
	int res;

	sleep(1);
	memset(pbuff, 0, 1024);
	
	sprintf(pbuff,"%s", "write this");
		
	
	//printf("'%s'\n", buff);
	res = write(fd, pbuff, strlen(pbuff)+1);
	if(res == strlen(pbuff)+1){
		fprintf(stderr, "Can not write to the device file.\n");		
		return (void*)-1;
	}	
	


	return (void*) 0;
}



int main(int argc, char **argv)
{
	int res;
	unsigned int test;
	unsigned int dumpid_open;
	unsigned int dumpid_read;
	unsigned int dumpid_write;
	//unsigned int dumpid_read_2;
	unsigned int dumpid_release;
	unsigned int second_dumpid_release;

	char buff[1024];
	int i = 0;

	if(argc == 1){
		return 0;
	}

	char* invalid_symbol = "totatlly_invalid";
	test = syscall(359, invalid_symbol, 0);
	if(test < 0) {
		fprintf(stderr,"Expected Value for invalid symbol\n");
		sleep(2);
	}

	char* symbol = "kbuf_driver_open";
	dumpid_open = syscall(359, symbol, 1);
	if(dumpid_open < 0) {
		fprintf(stderr, "ERROR=%d: syscall insdump\n", dumpid_open);
		exit(-1);
	}
	fprintf(stderr, "UVAR: dumpid_open = %u\n", dumpid_open);



	char* read_symbol = "kbuf_driver_read";
	dumpid_read = syscall(359, read_symbol, 0);
	if(dumpid_read < 0) {
		fprintf(stderr, "ERROR=%d: insdump read\n", dumpid_read);
		exit(-1);
	}

	char* release_symbol = "kbuf_driver_release";
	dumpid_release = syscall(359, release_symbol, 0);
	if(dumpid_release < 0) {
		fprintf(stderr, "ERROR: insdump release 1\n");
		exit(-1);
	}

	second_dumpid_release = syscall(359, release_symbol, 2);
	if(second_dumpid_release < 0) {
		fprintf(stderr, "ERROR: insdump release 2\n");
		exit(-1);
	}

	
	/*
	pid_t child_pid = fork();
	fprintf(stderr, "UVAR: child_pid = %d\n", child_pid);
	if(child_pid < 0) {
		fprintf(stderr, "UERROR: child_pid fork\n");
		exit(-1);
	}
	*/

	char* write_symbol = "kbuf_driver_write";
	dumpid_write = syscall(359, write_symbol, 1);
	if(dumpid_write < 0)
	{
		fprintf(stderr, "ERROR: kbuf_driver_write\n");
		exit(-1);
	}

	res = pthread_create(&p1, NULL, access_write, NULL);
	if(res < 0) {
		fprintf(stderr, "ERROR=%d: pthread_create\n", res);
		exit(-1);
	}



	/* open devices */
	fd = open("/dev/kbuf", O_RDWR);
	if (fd < 0 ){
		fprintf(stderr, "Can not open device file.\n");		
		return 0;
	}else{
		if(strcmp("show", argv[1]) == 0){
			memset(buff, 0, 1024);
			res = read(fd, buff, 256);
			sleep(1);
			// printf("'%s'\n", buff);
		}else if(strcmp("write", argv[1]) == 0){
			memset(buff, 0, 1024);
			if(argc >= 3){
				sprintf(buff,"%s", argv[2]);
				for(i = 3; i < argc; i++)
					sprintf(buff,"%s %s",buff,argv[i]);
			}
			//printf("'%s'\n", buff);
			res = write(fd, buff, strlen(buff)+1);
			if(res == strlen(buff)+1){
				fprintf(stderr, "Can not write to the device file.\n");		
				return 0;
			}	
		}
		/* close devices */
		pthread_join(p1, NULL);
		close(fd);
	}

	res = syscall(360, dumpid_open);
	if(res < 0) {
		fprintf(stderr, "ERROR=%d: rmdump\n", res);
		exit(-1);
	}
	fprintf(stderr, "UINFO: Removed dumpid for open\n");

	res = syscall(360, dumpid_read);
	if(res < 0) {
		fprintf(stderr, "ERROR=%d: rmdump\n", res);
		exit(-1);
	}
	fprintf(stderr, "UINFO; Removed dumpid for read\n");

	res = syscall(360, dumpid_release);
	if(res < 0) {
		fprintf(stderr, "ERROR: rmdump\n");
		exit(-1);
	}
	fprintf(stderr, "UINFO: Removed dumpid for first release\n");

	return 0;
}
