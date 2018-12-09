#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

#define BUFF_SIZE 1024

void *thrfunc(void* arg);

struct Info{
	int fd;
	bool* terminate;
};

struct RecvPacket{
	int client_num;
	char buf[BUFF_SIZE];
};

typedef struct Info info;
typedef struct RecvPacket recv_packet;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int fd;

int main()
{
	struct sockaddr_in s;
	bool terminate = false;
	char buf[BUFF_SIZE];
	int status;
	pthread_t tid;

	signal(SIGINT, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	bzero((char*)&s, sizeof(s));

	s.sin_family = AF_INET;
	s.sin_addr.s_addr = inet_addr("127.0.0.1");
	s.sin_port = htons(1234);

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if(fd < 0)
	{
		perror("simplex-talk: socket");
		exit(1);
	}

	if(connect(fd, (struct sockaddr*)&s, sizeof(s)) == -1)
	{
		perror("simplex-talk: connect");
		exit(1);
	}

	info server_info;
	server_info.fd = fd;
	server_info.terminate = &terminate;

	if((status = pthread_create(&tid, NULL, thrfunc, (void*)&server_info)) != 0) {
		printf("%lu thread create error: %s\n", tid, strerror(status));
		exit(0);
	}

	while(!terminate)
	{
		fgets(buf, sizeof(buf), stdin);

		if(!strcmp(buf, "Q\n"))
		{
			terminate = true;
		}
		send(fd, buf, sizeof(buf), 0);
	}

	close(fd);

	return 0;
}

void *thrfunc(void* arg)
{
	info* server_info = (info*)arg;

	int fd = server_info->fd;
	recv_packet* packet = (recv_packet*)malloc(sizeof(recv_packet));

	while(!*server_info->terminate)
	{
		recv(fd, packet, sizeof(recv_packet), 0);
		printf("%d Client : %s\n", packet->client_num, packet->buf);
	}

	return NULL;
}