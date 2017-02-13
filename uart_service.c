#include <stdio.h>
#include <string.h>

#include "uart_service.h"

static struct uart_service_cmd _cmds[UART_SERVICE_MAX_NR_CMD];

void uart_service_do(void)
{
	char *cmd = io_uart_cmd_get();
	if (cmd == NULL) {
		return;
	}

	int i;
	for (i=0; i<UART_SERVICE_MAX_NR_CMD; i++) {
		if (_cmds[i].cmd == NULL)
			continue;

		if (strncasecmp(_cmds[i].cmd, cmd, UART_SERVICE_MAX_LENGTH_CMD) !=0) {
			continue;
		}

		if (!_cmds[i].cb) {
			continue;
		}

		_cmds[i].cb(cmd);
		break;
	}
}

void uart_service_init(struct uart_service_cmd *cmds, int nr_cmds)
{
	memset(_cmds, 0, sizeof(_cmds));
	memcpy(_cmds, cmds, sizeof(struct uart_service_cmd) * nr_cmds);
}

