#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "io_uart.h"
#include "uart_queue.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"

#define UART_TX_BUF_SIZE	256
#define UART_RX_BUF_SIZE	256
#define UART_MAX_DATA_LEN	(UART_QUEUE_CMD_LEN + 1)

static void _uart_print_newline(void)
{
	uint8_t cr = '\r';
	app_uart_put(cr);
	cr = '\n';
	app_uart_put(cr);
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
	char error_msg[32] = {0,};
	static uint8_t index = 0;
	static char data_array[UART_MAX_DATA_LEN] = {0,};
	uint8_t cr;

	switch (p_event->evt_type) {
        case APP_UART_DATA_READY:
			app_uart_get(&cr);
			data_array[index++] = cr;
			app_uart_put(cr);

			if (cr == '\r') {
				_uart_print_newline();

				if (index == UART_MAX_DATA_LEN) {
					data_array[index-1] = 0;
					if (!uart_queue_enqueue(data_array)) {
						sprintf(error_msg, "ENQUEUE FAILED\r\n");
						io_uart_print(error_msg);
					}
#if 0
					else {
						char b[128] = {0,};

						data_array[3] = 0;
						sprintf(b, "data_array:%s\r\n", data_array);
						io_uart_print(b);

						char *aaa = uart_queue_dequeue();
						if (aaa != NULL) {
							memset(b, 0, sizeof(b));
							sprintf(b, "%x %x %x\r\n", aaa[0], aaa[1], aaa[2]);
							io_uart_print(b);

							memset(b, 0, sizeof(b));
							sprintf(b, "%p\r\n", &aaa[0]);
							io_uart_print(b);
						}
						else {
							sprintf(b, "NULL\r\n");
							io_uart_print(b);
						}
					}
#endif
				}
				index = 0;
			}
			else if (index >= UART_MAX_DATA_LEN) {
				index = 0;
			}
			break;
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;
        default:
            break;
	}

	/*
	//printf("\r\n uart_event_handle \r\n");
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if ((data_array[index - 1] == '\n') || (index >= (BLE_NUS_MAX_DATA_LEN)))
            {
                err_code = ble_nus_string_send(&m_nus, data_array, index);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }

                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
	*/
}

void io_uart_print(char *str)
{
	int i;
	for (i=0; i<strlen(str); i++) {
		app_uart_put(str[i]);
	}
}

char *io_uart_cmd_get(void)
{
	return uart_queue_dequeue();
}

void io_uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);

    printf("\r\n[UART] initialized!\r\n");
}

void io_uart_cleanup(void)
{
		app_uart_flush();
		app_uart_close();
}

