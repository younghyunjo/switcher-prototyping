#ifndef _UART_QUEUE_H_
#define _UART_QUEUE_H_

#include <stdbool.h>

#define UART_QUEUE_CMD_LEN	3

bool uart_queue_enqueue(char *cmd);
char* uart_queue_dequeue(void);

#endif
