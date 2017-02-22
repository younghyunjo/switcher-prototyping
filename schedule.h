#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#define SCHDULE_ID_UNKNOWN	0xff

struct schedule {
	uint8_t id;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t on;
};

uint8_t schedule_add(struct schedule *schedule);
void schedule_init(void);

#endif //#ifndef _SCHEDULE_H_
