#ifndef _IO_UART_H_
#define _IO_UART_H_

#include "uart_queue.h"

#define IO_UART_CMD_LEN UART_QUEUE_CMD_LEN

void io_uart_print(char *str);
char *io_uart_cmd_get(void);

void io_uart_init(void);
void io_uart_cleanup(void);


#endif //#ifndef _IO_UART_H_
