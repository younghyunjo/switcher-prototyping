#include "app_simple_timer.h"
#include "sdk_macros.h"
#include <stdio.h>
#include "nrf_drv_gpiote.h"

#define BUTTON_DETECTION_DELAY_US	50000

static void (*_release_cb)(void) = NULL;
static uint8_t _gpio;
static bool _btn_transition = false;
static bool _btn_state = false;

void _cb_timer_timeout(void* p_conect)
{
	if (_release_cb && _btn_transition && (_btn_state == nrf_drv_gpiote_in_is_set(_gpio))) {
		_release_cb();
	}
}

static void _button_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	app_simple_timer_stop();

	if (!_btn_transition) {
		_btn_state = nrf_drv_gpiote_in_is_set(_gpio);
		_btn_transition = true;

		app_simple_timer_start(APP_SIMPLE_TIMER_MODE_SINGLE_SHOT, _cb_timer_timeout, BUTTON_DETECTION_DELAY_US, NULL);
	}
	else {
		_btn_transition = false;
	}
}

uint32_t io_button_init(uint8_t gpio, void (*release_cb)(void))
{
    uint32_t err_code = app_simple_timer_init();
	VERIFY_SUCCESS(err_code);

    if (!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        VERIFY_SUCCESS(err_code);
    }

	nrf_drv_gpiote_in_config_t config = {
		.sense = NRF_GPIOTE_POLARITY_TOGGLE,
		.pull = NRF_GPIO_PIN_PULLUP,
		.is_watcher = false,
		.hi_accuracy = false,
	};
	err_code = nrf_drv_gpiote_in_init(gpio, &config, _button_event_handler);
	VERIFY_SUCCESS(err_code);

	nrf_drv_gpiote_in_event_enable(gpio, true);

	_gpio = gpio;
	_release_cb = release_cb;

	return NRF_SUCCESS;
}

void io_button_cleanup(void)
{
	app_simple_timer_stop();
	nrf_drv_gpiote_in_event_disable(_gpio);

	_gpio = 0xff;
	_btn_transition = false;
	_btn_state = false;
	_release_cb = NULL;
}
