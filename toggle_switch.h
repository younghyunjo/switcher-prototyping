#ifndef _TOGGLE_SWITCH_H_
#define _TOGGLE_SWITCH_H_

typedef void (*toggle_switch_push_cb_t)(void);

void toggle_switch_init(uint8_t pin_no, toggle_switch_push_cb_t cb);

#endif //#ifndef _TOGGLE_SWITCH_H_
