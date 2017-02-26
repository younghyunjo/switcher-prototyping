/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
/** @example examples/ble_peripheral/ble_app_hrs/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>

#include "battery.h"
#include "battery_service.h"
#include "bluetooth.h"
#include "current_time_service.h"
#include "device_information_service.h"
#include "hw_config.h"
#include "io_uart.h"
#include "motor.h"
#include "motor_service.h"
#include "now.h"
#include "schedule.h"
#include "schedule_service.h"
#include "toggle_switch.h"
#include "uart_service.h"

#include "nrf_delay.h"

#include "app_timer.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


//#define UART_ENABLE


#define APP_TIMER_PRESCALER              0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                           /**< Size of timer operation queues. */

#define DEAD_BEEF                        0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
	app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
	// Initialize timer module.
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
}


static void _battery_level_get_by_ble(void)
{
	battery_level_get(battery_service_level_update);
}

static void _switch_pushed(void)
{
	motor_move();
	NRF_LOG_INFO("SWITCH PUSHED\r\n");
}


static void _bat_level_print(uint8_t bat_level)
{
	char bat_level_str[32] = {0,};
	sprintf(bat_level_str, "BAT:%d\r\n", bat_level);
	io_uart_print(bat_level_str);
}
static void _uart_bat_cmd_do(char *cmd)
{
	battery_level_get(_bat_level_print);
}

static void _uart_swt_cmd_do(char *cmd)
{
	motor_move();
	NRF_LOG_INFO("SWITCH PUSHED\r\n");
}

static void _uart_sch_cmd_do(char *cmd)
{
	struct schedule schedules[SCHEDULE_MAX_NR];
	schedule_list(schedules);

	int i;
	char sch_str[32] = {0,};
	for (i=0; i<SCHEDULE_MAX_NR; i++) {
		if (schedules[i].id == SCHEDULE_ID_UNKNOWN) {
			continue;
		}

		memset(sch_str, 0, sizeof(sch_str));
		snprintf(sch_str, sizeof(sch_str), "id:%d day:%d hour:%d minute:%d\r\n",
				schedules[i].id,
				schedules[i].day,
				schedules[i].hour,
				schedules[i].minute);
		io_uart_print(sch_str);
	}
}

static void _uart_now_cmd_do(char *cmd)
{
	time_t current_time = now();

	char now_str[32] = {0,};
	ctime_r(&current_time, now_str);
	now_str[strlen(now_str)] = '\r';

	io_uart_print(now_str);
}

/**@brief Function for application main entry.
*/
int main(void)
{
	uint32_t err_code;

	// Initialize.
	err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);

	timers_init();

#ifdef UART_ENABLE
	io_uart_init();
#endif

	now_init();
	battery_init(BAT_APIN, BAT_MIN_LEVEL, BAT_MAX_LEVEL);
	motor_init(MOTOR_PIN, MOTOR_TR_PIN);
	toggle_switch_init(TOGGLE_SWITCH_PIN, _switch_pushed);

	schedule_init();
	now_event_handler_register(schedule_timer_evt_handler);

	bluetooth_init();

	battery_service_init(_battery_level_get_by_ble);
	device_information_service_init(MANUFACTURER_NAME);
	current_time_service_init(now, now_update);
	motor_service_init(motor_move);
	schedule_service_init();

	// Start execution.
	bluetooth_advertising_start();

	struct uart_service_cmd uart_service_cmds[] = {
		{.cmd = "bat", .cb = _uart_bat_cmd_do},
		{.cmd = "swt", .cb = _uart_swt_cmd_do},
		{.cmd = "now", .cb = _uart_now_cmd_do},
		{.cmd = "sch", .cb = _uart_sch_cmd_do},
	};
	uart_service_init(uart_service_cmds, ARRAY_SIZE(uart_service_cmds));

#if 0
	{
		struct schedule s;
		s.id = 0xff;
		s.day = 4;
		s.hour = 0;
		s.minute = 1;
		schedule_add(&s);
	}
#endif

	{
		/*
		nrf_delay_ms(2000);
		schedule_del(3);
		nrf_delay_ms(2000);
		schedule_del(5);
		nrf_delay_ms(2000);
		schedule_del(9);
		nrf_delay_ms(2000);

		struct schedule s[SCHEDULE_MAX_NR];
		schedule_list(s);
		NRF_LOG_INFO("id:%d day:%d hour:%d minute:%d\r\n",
				s[0].id, s[0].day, s[0].hour, s[0].minute);
		*/
	}

	// Enter main loop.
	for (;;)
	{
#ifdef UART_ENABLE
		__WFE();
		uart_service_do();
#else
		if (NRF_LOG_PROCESS() == false) {
			__WFE();
		}
#endif
	}
}

