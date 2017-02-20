#ifndef _BATTERY_H_
#define _BATTERY_H_

void battery_level_get(void (*button_level_cb)(uint8_t));
void battery_init(uint8_t battery_pin, uint16_t min_voltage_x1000, uint16_t max_voltage_x1000);
void battery_cleanup(void);

#endif  //#ifndef _BATTERY_H_
