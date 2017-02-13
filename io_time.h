#ifndef _IO_TIME_H_
#define _IO_TIME_H_

void io_time_set(time_t update_time);
time_t io_time_get(void);

void io_time_init(void);
void io_time_cleanup(void);

#endif //#ifndef _IO_TIME_H_
