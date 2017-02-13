#include "nrf_gpio.h"
#include "app_pwm.h"
#include "nrf_delay.h"

//#define MOTOR_DISABLE


#define IO_MOTOR_TR_NO_PIN 		0xff
#define SG90_PERIOD_US			20000
#define SG90_PLUS_90_DUTY		10
#define SG90_MINUS_90_DUTY		5
#define SG90_MOVING_DELAY_MS	(100 * 180 / 60)

#define _motor_on()	do { \
		nrf_gpio_pin_set(_tr_pin_no); \
		app_pwm_enable(&PWM1); \
	} while (0);
#define _motor_off() do {\
		app_pwm_disable(&PWM1); \
		nrf_gpio_pin_clear(_tr_pin_no); \
	} while (0);
#define _motor_move(duty)	do { \
		while(app_pwm_channel_duty_set(&PWM1, 0, duty)  == NRF_ERROR_BUSY); \
		nrf_delay_ms(SG90_MOVING_DELAY_MS); \
	} while (0);
		
#define _motor_plus_90_move() _motor_move(SG90_PLUS_90_DUTY)
#define _motor_minus_90_move() _motor_move(SG90_MINUS_90_DUTY)

static uint8_t _tr_pin_no = IO_MOTOR_TR_NO_PIN;

static APP_PWM_INSTANCE(PWM1, 1);                   // Create the instance "PWM1" using TIMER1.

void io_motor_init(uint8_t motor_pin_no, uint8_t tr_pin_no)
{
#ifdef MOTOR_DISABLE
	return;
#endif

	app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(SG90_PERIOD_US, motor_pin_no);
	pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH; 
	ret_code_t  err_code = app_pwm_init(&PWM1, &pwm1_cfg, NULL);
	APP_ERROR_CHECK(err_code);

	_tr_pin_no = tr_pin_no;
	nrf_gpio_cfg_output(_tr_pin_no);

	_motor_on();
	_motor_plus_90_move();
	_motor_off();

	printf("[MTR] intialized\r\n");
}

void io_motor_cleanup(void)
{
	_motor_off();
	app_pwm_uninit(&PWM1);
	_tr_pin_no = IO_MOTOR_TR_NO_PIN;
}

void io_motor_move(void)
{
#ifdef MOTOR_DISABLE
	return;
#endif
	if (_tr_pin_no == IO_MOTOR_TR_NO_PIN) {
		return;
	}

	_motor_on();
	_motor_minus_90_move();
	_motor_plus_90_move();
	_motor_off();
}
