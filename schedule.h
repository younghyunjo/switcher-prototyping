#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

void sch_test(void);

struct schedule_info {
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t on;
};

void schedule_add(struct schedule_info *schedule_info);
void schedule_init(void);

#endif //#ifndef _SCHEDULE_H_
