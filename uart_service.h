#ifndef _UART_SERVICE_H_
#define _UART_SERVICE_H_

#include "io_uart.h"

#define UART_SERVICE_MAX_LENGTH_CMD	IO_UART_CMD_LEN
#define UART_SERVICE_MAX_NR_CMD		5

struct uart_service_cmd {
	char cmd[UART_SERVICE_MAX_LENGTH_CMD + 1];
	void (*cb)(char *cmd);
};


void uart_service_do(void);
void uart_service_init(struct uart_service_cmd *cmds, int nr_cmds);

#endif //#ifndef _UART_SERVICE_H_
