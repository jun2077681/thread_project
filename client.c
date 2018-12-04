#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

int main()
{
	struct sockaddr_in s;
	bzero((char*)&s, sizeof(s));

	s.sin_family = AF_INET;
	s.sin_addr.s_addr = inet_addr("127.0.0.1");
	s.sin_port = htons(1234);

	int fd = socket(PF_INET, SOCK_STREAM, 0);
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


}