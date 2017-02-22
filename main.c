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
#include "motor.h"
#include "motor_service.h"
#include "now.h"
#include "schedule.h"
#include "toggle_switch.h"

#include "app_timer.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


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

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void power_manage(void)
{
	uint32_t err_code = sd_app_evt_wait();

	APP_ERROR_CHECK(err_code);
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

/**@brief Function for application main entry.
*/
int main(void)
{
	uint32_t err_code;

	// Initialize.
	err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);

	timers_init();

	now_init();
	battery_init(BAT_APIN, BAT_MIN_LEVEL, BAT_MAX_LEVEL);
	motor_init(MOTOR_PIN, MOTOR_TR_PIN);
	toggle_switch_init(TOGGLE_SWITCH_PIN, _switch_pushed);

	schedule_init();

	bluetooth_init();

	battery_service_init(_battery_level_get_by_ble);
	device_information_service_init(MANUFACTURER_NAME);
	current_time_service_init(now, now_update);
	motor_service_init(motor_move);

	// Start execution.
	bluetooth_advertising_start();


#if 0
	{
		struct schedule s;
		s.id = 0xff;
		s.day = 1;
		s.hour = 12;
		s.minute = 25;
		s.on = 1;

		schedule_add(&s);
	}
#endif

	// Enter main loop.
	for (;;)
	{
		if (NRF_LOG_PROCESS() == false)
		{
			power_manage();
		}
	}
}

