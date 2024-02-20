#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "saferq.h"

message_saferqueue *pingq;
message_saferqueue *pongq;

void* pingpong(void *queue) 
{
	int i;
	char *message;
	message_saferqueue *q = (message_saferqueue*)queue;

	for (i = 0; i < 10; ++i) {
		message = message_dequeue(q);
		printf("Message %d: %s\n", i+1, message);
		free(message);
	}

	pthread_exit(0);
}

int main()
{
	char *message, *number;
	int i;
	pthread_t ping_t, pong_t;

	pingq = (message_saferqueue *)malloc(sizeof(message_saferqueue));
	pingq->queue = (message_queue*)malloc(sizeof(message_queue));
	pthread_mutex_init(&(pingq->lock), NULL);
	pthread_cond_init(&(pingq->cond), NULL);

	pongq = (message_saferqueue *)malloc(sizeof(message_saferqueue));
	pongq->queue = (message_queue*)malloc(sizeof(message_queue));
	pthread_mutex_init(&(pongq->lock), NULL);
	pthread_cond_init(&(pongq->cond), NULL);

	pthread_create(&ping_t, NULL, pingpong, (void *)pingq);
	pthread_create(&pong_t, NULL, pingpong, (void *)pongq);

	for (i = 0; i < 20; ++i) {
		number = (char *)malloc(4);
		sprintf(number, " %d", i+1);
		message = (char *)malloc(MESSAGE_SIZE);

		if (i % 2 == 0) {
			strcpy(message, "ping");
			strcat(message, number);
			message_enqueue(pingq, message);
		} else {
			strcpy(message, "pong");
			strcat(message, number);
			message_enqueue(pongq, message);
		}

		free(number);
		free(message);
	}
	pthread_join(ping_t, NULL);
	pthread_join(pong_t, NULL);

	return 0;
}
