#include "app_timer.h"

#define APP_TIMER_PRESCALER              0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TICK_EVENT_INTERVAL     APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) /**< 500 miliseconds tick event interval in timer tick units. */

static void (*_handler)(time_t now) = NULL;
static time_t global_time = 0;
APP_TIMER_DEF(m_timer);

static void _timer_evt_handler(void * p_context)
{
	global_time += 1;
	if (_handler)
		_handler(global_time);
}

void now_update(time_t update_time)
{
	global_time = update_time;
}

time_t now(void)
{
	return global_time;
}

void now_event_handler_register(void (*handler)(time_t now))
{
	_handler = handler;
}

void now_init(void)
{
    ret_code_t err_code = app_timer_create(&m_timer,
                                APP_TIMER_MODE_REPEATED,
                                _timer_evt_handler);

    err_code = app_timer_start(m_timer, APP_TICK_EVENT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
