#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "saferq.h"

void message_enqueue(message_saferqueue *q, const char *m) {
	/* 1. Point n's prev to tail; point n's next to NULL
	 * 2. Point tail's next to n; leave tail's prev alone
	 * 3. Point q's tail to n
	 * 4. Increment q's counter
	 */

	message_node *n = (message_node*)malloc(sizeof(message_node));
	strncpy(n->message, m, MESSAGE_SIZE); /* (*n).message */
	n->message[MESSAGE_SIZE - 1] = '\0';

	/** CRITICAL REGION:
	 * 1. Acquire q's lock
	 * 2. Do work (update q's queue)
	 * 3. Post to q's condition variable (work completed)
	 * 4. Release q's lock
	 */

	/* 1. Acquire q's lock */
	pthread_mutex_lock(&(q->lock));

	/* 2. Do work: point n's prev to tail; point n's next to NULL */
	n->prev = q->queue->tail;
	n->next = NULL;
	
	/* 2. Do work: point tail's next to n; leave tail's prev alone
	 * 2. a. if queue is empty: point head to new node n
	 */
	if (q->queue->count > 0) {
		q->queue->tail->next = n;
	} else {
		q->queue->head = n;
	}

	/* 2. Do work: point queue's tail to n */
	q->queue->tail = n;

	/* 2. Do work: update queue's count */
	q->queue->count += 1;

	/* 3. Post to q's condition variable to signal that work is complete */
	pthread_cond_signal(&(q->cond));

	/* 4. Release q's lock */
	pthread_mutex_unlock(&(q->lock));
}

char* message_dequeue(message_saferqueue *q) {
	char *message = (char *)malloc(MESSAGE_SIZE);

	/** CRITICAL REGION \
	 * 1. Acquire q's lock
	 * 2. Wait on q's condition variable--if no work (i.e., no messages in queue), go to sleep
	 * 3. When past the condition variable: double-check there's work--if not, back to sleep
	 * 4. Do work.
	 * 5. Decrement q's counter
	 * 6. Release q's lock
	 */
	
	/* 1. Acquire q's lock */
	pthread_mutex_lock(&(q->lock));

	/* 2. Wait on q's condition variable--checking if any message available to work on */
	while (q->queue->count == 0) {
		/* 3. if in here: need to double-check count before exiting */
		pthread_cond_wait(&(q->cond), &(q->lock));
	}

	/* 4. Do work: extract the message */
	strncpy(message, q->queue->head->message, MESSAGE_SIZE);
	
	/* 4. Do work: remove the old head node */
	if (q->queue->count > 1) {
		/* Update q's head to point to the new head node */
		q->queue->head = q->queue->head->next;
		/* Delete the old head node */
		free(q->queue->head->prev);
		/* Break the new head node's link to the old head node */
		q->queue->head->prev = NULL;
	} else { /* q->queue->count == 1 */
		free(q->queue->head);
		q->queue->head = NULL;
		q->queue->tail = NULL;
	}

	/* 5. Decrement q's counter */
	q->queue->count -= 1;

	/* 6. Release q's lock */
	pthread_mutex_unlock(&(q->lock));

	return message;
}
