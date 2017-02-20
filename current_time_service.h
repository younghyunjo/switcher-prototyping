#ifndef _CURRENT_TIME_SERVICE_H_
#define _CURRENT_TIME_SERVICE_H_

#include <time.h>

void current_time_service_evt_handler(ble_evt_t *p_ble_evt);
uint32_t current_time_service_init(time_t (*current_time_get)(void), void (*current_time_set)(time_t current_time));

#endif //#ifndef _CURRENT_TIME_SERVICE_H_
