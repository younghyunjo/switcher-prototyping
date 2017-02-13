#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "boards.h"
#include "app_error.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define RTC_CHANNEL	0
#define RTC_PRESCALER_FOR_125MS_TICK	4095
#define RTC_NR_TICK_1SEC	(1UL * 8)

static const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(1);
static time_t global_time = 0;

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
	if (int_type != NRF_DRV_RTC_INT_COMPARE0) {
		return;
	}

	global_time += 1;

	uint32_t cc = nrf_rtc_cc_get(rtc.p_reg, RTC_CHANNEL);
	nrf_drv_rtc_cc_set(&rtc, RTC_CHANNEL, cc + RTC_NR_TICK_1SEC, true);

#if 1
	printf("[TIME] %d\r\n", int_type);
	printf("%ld\r\n", nrf_rtc_counter_get(rtc.p_reg));
#endif
}

void io_time_set(time_t update_time)
{
	global_time = update_time;
}

time_t io_time_get(void)
{
	return global_time;
}

void io_time_init(void)
{
	nrf_drv_rtc_config_t config = {
		.prescaler          = RTC_PRESCALER_FOR_125MS_TICK,
		.interrupt_priority = RTC_DEFAULT_CONFIG_IRQ_PRIORITY,
		.reliable           = RTC_DEFAULT_CONFIG_RELIABLE,
		.tick_latency       = RTC_US_TO_TICKS(NRF_MAXIMUM_LATENCY_US, RTC_DEFAULT_CONFIG_FREQUENCY),
	};
    uint32_t err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_rtc_cc_set(&rtc, RTC_CHANNEL, RTC_NR_TICK_1SEC, true);
    APP_ERROR_CHECK(err_code);

    nrf_drv_rtc_enable(&rtc);
}

void io_time_cleanup(void)
{
	nrf_drv_rtc_cc_disable(&rtc, RTC_CHANNEL);
	nrf_drv_rtc_disable(&rtc);
	nrf_drv_rtc_uninit(&rtc);
}

