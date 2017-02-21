#ifndef _MOTOR_SERVICE_H_
#define _MOTOR_SERVICE_H_

void motor_service_evt_handler(ble_evt_t *p_ble_evt);
uint32_t motor_service_init(void (*motor_move)(void));

#endif //#ifndef _MOTOR_SERVICE_H_


