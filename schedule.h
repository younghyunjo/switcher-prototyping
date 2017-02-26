#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#define SCHEDULE_ID_UNKNOWN	0xff
#define SCHEDULE_MAX_NR 	10

struct schedule {
	uint8_t id;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
};

void schedule_list(struct schedule *schedule);
void schedule_del(uint8_t id);
uint8_t schedule_add(struct schedule *schedule);

void schedule_timer_evt_handler(time_t now);
void schedule_init(void);

#endif //#ifndef _SCHEDULE_H_
