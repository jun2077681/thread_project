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

struct listNode {
	int data;
	pthread_t tid;
	struct listNode *nextPtr;
};

struct RecvPacket{
	int client_num;
	char buf[BUFF_SIZE];
};

typedef struct listNode ListNode;
typedef struct RecvPacket recv_packet;
typedef ListNode *ListNodePtr;

ListNodePtr insert(ListNodePtr *sPtr, int value);
void delete(ListNodePtr *sPtr, int value);
bool isEmpty(ListNodePtr sPtr);
void printList(ListNodePtr currentPtr);

void *thrfunc(void* arg);

ListNodePtr accp_sock = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main()
{
	int server_socket;
	int sock;

	struct sockaddr_in server_addr, client_addr;
	int status;

	signal(SIGINT, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	bzero((char*)&server_addr, sizeof(server_addr));
	bzero((char*)&client_addr, sizeof(client_addr));

	server_socket = socket(PF_INET, SOCK_STREAM, 0);
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

	if(listen(server_socket, 10) < 0)
	{
		perror("Server : Can't listening connect.");
		exit(1);
	}

	while(1)
	{
		int addr_size = sizeof(client_addr);
		int sock = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);

		if(server_socket < 0)
		{
			perror("Server: accept failed.");
			exit(1);
		}

		ListNodePtr socknode = insert(&accp_sock, sock);

		ListNodePtr curPtr = accp_sock;
		
		recv_packet* packet = (recv_packet*)malloc(sizeof(recv_packet));
		packet->client_num = socknode->data;

		while(curPtr != NULL)
		{
			if(curPtr->data == socknode->data)
			{
				curPtr = curPtr->nextPtr;
				continue;
			}
			sprintf(packet->buf, "Client %d has been connected!\n", socknode->data);

			send(curPtr->data, packet, sizeof(recv_packet), 0);
			printf("-------------------------------------\n");
			printf("send to %d\n", curPtr->data);
			printf("send data : %s\n", packet->buf);
			printf("-------------------------------------\n");

			curPtr = curPtr->nextPtr;
		}

		if((status = pthread_create(&(socknode->tid), NULL, thrfunc, (void*)socknode)) != 0) {
			printf("%lu thread create error: %s\n", socknode->tid, strerror(status));
			exit(0);
		}
		pthread_detach(socknode->tid);
	}
}

void *thrfunc(void* arg)
{
	ListNodePtr socknode = (ListNodePtr)arg;
	int sock = socknode->data;
	bool terminate = false;
	char buf[BUFF_SIZE];

	while(!terminate)
	{
		recv(sock, buf, sizeof(buf), 0);
		printf("client %d send value = %s\n", sock, buf);

		pthread_mutex_lock(&lock);

		ListNodePtr curPtr = accp_sock;

		recv_packet* packet = (recv_packet*)malloc(sizeof(recv_packet));
		packet->client_num = sock;

		while(curPtr != NULL)
		{
			if(curPtr->data == sock)
			{
				curPtr = curPtr->nextPtr;
				continue;
			}

			memcpy(packet->buf, buf, sizeof(buf));

			send(curPtr->data, packet, sizeof(recv_packet), 0);
			printf("-------------------------------------\n");
			printf("send to %d\n", curPtr->data);
			printf("send data : %s\n", packet->buf);
			printf("-------------------------------------\n");

			curPtr = curPtr->nextPtr;
		}

		pthread_mutex_unlock(&lock);

		if(!strcmp(buf, "Q\n"))
		{
			terminate = true;
		}
	}
	close(sock);

	delete(&accp_sock, sock);

	return NULL;
}

ListNodePtr insert(ListNodePtr *sPtr, int value)
{
	ListNodePtr newPtr;
	ListNodePtr previousPtr;
	ListNodePtr currentPtr;

	newPtr = malloc(sizeof(ListNode));

	if(newPtr != NULL)
	{
		newPtr->data = value;
		newPtr->nextPtr = NULL;

		previousPtr = NULL;
		currentPtr = *sPtr;

		while(currentPtr != NULL)
		{
			previousPtr = currentPtr;
			currentPtr = currentPtr->nextPtr;
		}

		if(previousPtr == NULL)
		{
			newPtr->nextPtr = *sPtr;
			*sPtr = newPtr;
		}
		else
		{
			previousPtr->nextPtr = newPtr;
			newPtr->nextPtr = currentPtr;
		}
	}
	else
	{
		return NULL;
	}

	return newPtr;
}

void delete(ListNodePtr *sPtr, int value)
{
	ListNodePtr previousPtr;
	ListNodePtr currentPtr;
	ListNodePtr tempPtr;

	if(value == (*sPtr)->data)
	{
		tempPtr = *sPtr;
		*sPtr = (*sPtr)->nextPtr;
		free(tempPtr);
		return;
	}
	else
	{
		previousPtr = *sPtr;
		currentPtr = (*sPtr)->nextPtr;

		while(currentPtr != NULL && currentPtr->data != value)
		{
			previousPtr = currentPtr;
			currentPtr = currentPtr->nextPtr;
		}
		if(currentPtr != NULL)
		{
			tempPtr = currentPtr;
			previousPtr->nextPtr = currentPtr->nextPtr;
			free(tempPtr);
			return;
		}
	}

	return;
}

bool isEmpty(ListNodePtr sPtr)
{
	return sPtr == NULL;
}

void printList(ListNodePtr currentPtr)
{
	if(currentPtr == NULL)
	{
		printf("empty.\n");
	}
	else
	{
		printf("List : \n");
		while(currentPtr != NULL)
		{
			printf("fd : %d tid : %lu\n", currentPtr->data, currentPtr->tid);
			currentPtr = currentPtr->nextPtr;
		}
		printf("NULL\n\n");
	}
}