#include <stdio.h>

#include "app_error.h"
#include "nrf_drv_adc.h"
#include "nrf_adc.h"

#define ADC_BUFFER_SIZE 1
#define ADC_MAX_VOLTAGE	3.6

static nrf_adc_value_t _adc_buffer[ADC_BUFFER_SIZE];
static nrf_drv_adc_channel_t m_channel_config;

static uint16_t _min_voltage_x1000;
static uint16_t _max_voltage_x1000;
static void (*_button_level_cb)(uint8_t) = NULL;

static uint8_t _battery_calc(uint16_t adc_value)
{
	uint8_t level = 0;

	if (adc_value >= _max_voltage_x1000 / ADC_MAX_VOLTAGE) {
		level = 100;
	}
	else if (adc_value <= _min_voltage_x1000 / ADC_MAX_VOLTAGE) {
		level = 0;
	}
	else {
		uint16_t scaled_max = (_max_voltage_x1000 - _min_voltage_x1000) / ADC_MAX_VOLTAGE ;
		uint16_t scaled_adc_value = adc_value - (_min_voltage_x1000 / ADC_MAX_VOLTAGE);
		level = 100 * scaled_adc_value / scaled_max;
	}

	return level;
}

static void _adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
	if (p_event->type != NRF_DRV_ADC_EVT_DONE) {
		return;
	}

	uint16_t adc_value = p_event->data.done.p_buffer[0];
	uint8_t level = _battery_calc(adc_value);

	if (_button_level_cb) 
		_button_level_cb(level);
	_button_level_cb = NULL;

}

void battery_level_get(void (*button_level_cb)(uint8_t))
{
	if (_button_level_cb) {
		return;
	}

	_button_level_cb = button_level_cb;

	nrf_drv_adc_buffer_convert(_adc_buffer, ADC_BUFFER_SIZE);
	nrf_drv_adc_sample();
}

void battery_init(uint8_t battery_pin, uint16_t min_voltage_x1000, uint16_t max_voltage_x1000)
{
	ret_code_t ret_code;

	nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;
	ret_code = nrf_drv_adc_init(&config, _adc_event_handler);
	APP_ERROR_CHECK(ret_code);

	nrf_drv_adc_channel_config_t adc_channel_config = {
		.resolution = NRF_ADC_CONFIG_RES_10BIT,
		.input = NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD,
		.reference = NRF_ADC_CONFIG_REF_VBG,
		.ain = 1 << battery_pin
	};
	m_channel_config.config.config = adc_channel_config;
	m_channel_config.p_next = NULL;
	nrf_drv_adc_channel_enable(&m_channel_config);

	_min_voltage_x1000 = min_voltage_x1000;
	_max_voltage_x1000 = max_voltage_x1000;
}

void battery_cleanup(void)
{
	nrf_drv_adc_channel_disable(&m_channel_config);
	nrf_drv_adc_uninit();
}

