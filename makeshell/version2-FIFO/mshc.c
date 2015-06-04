#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define SZ_STR_BUF		256

int ifd, ofd;

int send_to_server();
int recv_from_server();

void err_exit(char *msg){
	perror(msg);
	exit(1);
}

void connect(void){
	char *fifo_ctos = "fifo_ctos";
	char *fifo_stoc = "fifo_stoc";

	if((ofd = open(fifo_ctos, O_WRONLY)) < 0)
		err_exit(fifo_ctos);
	if((ifd = open(fifo_stoc, O_RDONLY)) < 0)
		err_exit(fifo_stoc);

	printf("Connected to server.\n");
	recv_from_server();
}

void single_client(void){
	while(1){
		recv_from_server();
		if(send_to_server() < 0)
			break;
		if(recv_from_server() < 0)
			break;
	}
	close(ofd);
	close(ifd);
}

int send_to_server(void){
	char cmd_line[SZ_STR_BUF];
	int len;

	if((len = read(0, cmd_line, SZ_STR_BUF)) <= 0)
		return(-1);
	if(write(ofd, cmd_line, len) != len)
		return(-1);
	return(0);
}

int recv_from_server(void){
	char cmd_line[SZ_STR_BUF];
	int len;

	if((len = read(ifd, cmd_line, SZ_STR_BUF)) <= 0)
		return(-1);
	if(write(1, cmd_line, len) != len)
		return(-1);
	return(0);
}

void *SendToServerThread(void *arg){
	while(1){
		if(send_to_server() < 0)
			break;
	}		
	close(ofd);
	return(NULL);
}

void *RecvFromServerThread(void *arg){
	while(1){
		if(recv_from_server() < 0)
			break;
	}
	close(ifd);
	return(NULL);
}

void thread_err_exit(int err, char *msg){
	printf("%s: %s\n", msg, strerror(err));
	exit(1);
}

void thread_client(void){
	int ret;
	pthread_t tid;

	if((ret = pthread_create(NULL, NULL, SendToServerThread, NULL)) != 0)
		thread_err_exit(ret, "pthread_create");
	if((ret = pthread_create(&tid, NULL, RecvFromServerThread, NULL)) != 0)
		thread_err_exit(ret, "pthread_create");
	pthread_join(tid, NULL);
}

int main(int argc, char *argv[]){
	connect();
//	single_client();
	thread_client();
	return(0);
}
