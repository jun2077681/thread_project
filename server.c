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

#define BUFF_SIZE 1024
#define THREAD_NUM 10

typedef struct _node{
	int value;
	struct _node* next;
}node;

typedef node* nptr;

typedef struct _list{
	int count;
	nptr head;
}list;

list* init(void);
void insert(list* lptr, int value);
void delete(list* lptr, int value);

int cntNum = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *thrfunc(void *arg);

int main()
{
	int server_socket, client_socket;
	int accp_sock[THREAD_NUM];
	struct sockaddr_in server_addr, client_addr;
	pthread_t tid[THREAD_NUM];
	int status;

	char buff_rcv[BUFF_SIZE+5];
	char buff_snd[BUFF_SIZE+5];

	bzero((char*)&server_addr, sizeof(server_addr));
	bzero((char*)&client_addr, sizeof(client_addr));

	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server_socket < 0)
	{
		perror("Server : Can't open stream socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(1234);

	if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Server : Can't bind local address.");
		exit(1);
	}

	while(1)
	{
		if(listen(server_socket, 10) < 0)
		{
			perror("Server : Can't listening connect.");
			exit(1);
		}

		int addr_size = sizeof(client_addr);
		accp_sock[cntNum] = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
		if(client_socket < 0)
		{
			perror("Server: accept failed.");
			exit(1);
		}
		if((status = pthread_create(&tid[cntNum], NULL, &thrfunc, (void *) &accp_sock[cntNum])) != 0) {
			printf("%d thread create error: %s\n", cntNum, strerror(status));
			exit(0);
		}

		pthread_join(tid[cntNum], NULL);
		cntNum++;
		if(cntNum == 10) cntNum = 0;
	}	
}

void *thrfunc(void *arg) {
	int accp_sock = (int) *((int*) arg);
	int buf;

	read(accp_sock, &buf, 4);
	printf("client send value = %d\n", buf);
	pthread_mutex_lock(&lock);
	result += buf;
	printf("result = %d\n", result);
	pthread_mutex_unlock(&lock);
	write(accp_sock, &result, 4);

	close(accp_sock);
}

list* init(void)
{
	list* mylist = (list*)malloc(sizeof(list));
	mylist->count = 0;
	mylist->head = NULL;

	return mylist;
}

void insert(list* lptr, int value)
{
	nptr new_nptr = (node*)malloc(sizeof(node));
	new_nptr->value = value;
	new_nptr->next = NULL;

	if(lptr->head == NULL)
	{
		lptr->head = new_nptr;
	}
	else
	{
		nptr tail = lptr->head;
		while(tail->next != NULL)
		{
			tail = tail->next;
		}
		tail->next = new_nptr;
	}
	lptr->count++;
}
void delete(list* lptr, int value)
{
	if(lptr->count == 0) return;
	if(lptr->count == 1)
	{
		if(lptr->head->value == value)
		{
			nptr tmp = lptr->head;
			lptr->head = NULL;
			free(tmp);
			lptr->count--;
		}
		else
		{
			return;
		}
	}
	nptr cur = lptr->head;

	while(cur != NULL && cur->next!=NULL && cur->next->value != value)
	{
		cur = cur->next;
	}
	if(cur == NULL || cur->next == NULL) return;

	nptr tmp = cur->next;
	cur->next = tmp->next;
	free(tmp);
	lptr->count--;

	return;
}