#ifndef _IO_BUTTON_H_
#define _IO_BUTTON_H_

uint32_t io_button_init(uint8_t pin_no, void (*io_button_release_cb)(void));
void io_button_cleanup(void);

#endif //#ifndef _IO_BUTTON_H_
