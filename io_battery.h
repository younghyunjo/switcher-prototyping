#ifndef _IO_BATTERY_H_
#define _IO_BATTERY_H_

void io_battery_level(void (*button_level_cb)(uint16_t));

void io_battery_init(uint8_t battery_pin, uint16_t min_voltage_x1000, uint16_t max_voltage_x1000);
void io_battery_cleanup(void);

#endif  //#ifndef _IO_BATTERY_H_
