#ifndef _NOW_H_
#define _NOW_H_

#include <time.h>

/*
 * @brief update current time
 */
void now_update(time_t update_time);

/*
 * @brief current time get
 */
time_t now(void);

/*
 * @brief initialization current time
 */
void now_init(void);

#endif //#ifndef _NOW_H_
