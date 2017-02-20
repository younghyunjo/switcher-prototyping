#include <stdint.h>

#include "app_button.h"
#include "app_timer.h"
#include "toggle_switch.h"

#define APP_TIMER_PRESCALER		0
#define DETECTION_DELAY			APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)


#define TOGGLE_SWITCH_PIN_DUMMY 0xff

static uint8_t _pin_no = TOGGLE_SWITCH_PIN_DUMMY;
static toggle_switch_push_cb_t _release_cb = NULL;

static void _event_handler(uint8_t pin_no, uint8_t action)
{
	if (pin_no != _pin_no) {
		return;
	}

	if (action == APP_BUTTON_RELEASE && _release_cb) {
			_release_cb();
	}
}

void toggle_switch_init(uint8_t pin_no, toggle_switch_push_cb_t cb)
{
	uint32_t err_code;

	_pin_no = pin_no;
	_release_cb = cb;

	static app_button_cfg_t buttons[] = {
		{0xff, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, _event_handler},
	};
	buttons[0].pin_no = _pin_no;

	err_code = app_button_init(buttons, ARRAY_SIZE(buttons), DETECTION_DELAY);
	APP_ERROR_CHECK(err_code);

	err_code = app_button_enable();
	APP_ERROR_CHECK(err_code);

	printf("[TOGGLE_SWITCH] initialized\r\n");
}
