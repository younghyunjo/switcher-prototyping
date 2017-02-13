#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart_queue.h"

#define UART_QUEUE_NR_CMDS	4
#define UART_QUEUE_LEN		((UART_QUEUE_CMD_LEN+1) * UART_QUEUE_NR_CMDS)

static char _queue[UART_QUEUE_LEN] = {0,};
static int8_t front = -1;
static int8_t rear = -1;

static inline bool _uart_queue_full(void)
{
	if ((front == 0 && rear == (UART_QUEUE_LEN-1)) || (front == rear +1))
		return true;

	return false;
}

static inline bool uart_queue_empty(void)
{
	if ((rear == -1 && front == -1) || front == rear)
		return true;

	return false;
}

bool uart_queue_enqueue(char *cmd)
{
	if (strlen(cmd) != UART_QUEUE_CMD_LEN) {
		//command length must be 3
		return false;
	}

	if (_uart_queue_full()) {
		//queue is full
		return false;
	}

	bool queue_empty = uart_queue_empty();

	memcpy(&_queue[++rear], cmd, UART_QUEUE_CMD_LEN);
	rear += UART_QUEUE_CMD_LEN;
	_queue[rear] = 0;

	if (queue_empty) {
		front++;
		front = front % UART_QUEUE_LEN;
	}

	if (rear == UART_QUEUE_LEN-1)
		rear = -1;

	return true;
}

char* uart_queue_dequeue(void) 
{
	if (uart_queue_empty()) {
		return NULL;
	}

	int8_t old_front = front;

	front += (UART_QUEUE_CMD_LEN +1);

	if (front == UART_QUEUE_LEN)
		front = 0;

	if (front-1 == rear)  {
		front = rear;
	}

	return &_queue[old_front];
}

/*
void uart_queue_print(void)
{
	while (front != rear) {
		printf("%c ", _queue[front++]);
		front = front % UART_QUEUE_LEN;
	}
}
*/
