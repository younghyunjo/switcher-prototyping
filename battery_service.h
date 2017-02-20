#ifndef _BATTERY_SERVICE_H_
#define _BATTERY_SERVICE_H_

#include "ble.h"

//void battery_service_db_disc_evt(ble_db_discovery_evt_t * p_evt);
void battery_service_evt_handler(ble_evt_t *p_ble_evt);
void battery_service_level_update(uint8_t level);
void battery_service_init(void (*cb_connect)(void));

#endif //#ifndef _BATTERY_SERVICE_H_

