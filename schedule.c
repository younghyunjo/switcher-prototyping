#include <stdint.h>
#include <stdbool.h>

#include "app_error.h"
#include "fds.h"
#include "schedule.h"

#define NRF_LOG_MODULE_NAME "SCH"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define SCHEDULE_FILE_ID			0x01
#define SCHEDULE_MAX_NR 			10

#define INDEX_TO_RECORD_KEY(x)		((x)+1)

struct schedule_db_schema {
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t on;
};
static struct schedule_db_schema db_schema __ALIGN(4);

struct schedule_desc {
	bool loaded;
	struct {
		bool empty;
		struct schedule schedule;
	} schedules[SCHEDULE_MAX_NR];
};
static struct schedule_desc schedule_desc;

static ret_code_t _schedule_read(uint8_t record_key, struct schedule *schedule)
{
	fds_record_desc_t desc;
	memset(&desc, 0, sizeof(fds_record_desc_t));

	fds_find_token_t ft;
	memset(&ft, 0, sizeof(fds_find_token_t));

	ret_code_t err_code = fds_record_find(SCHEDULE_FILE_ID, record_key, &desc, &ft);
	if (err_code != FDS_SUCCESS) {
		if (err_code != FDS_ERR_NOT_FOUND) {
			NRF_LOG_ERROR("fds_record_find error:%d\r\n", err_code);
			return err_code;
		}

		NRF_LOG_INFO("%d empty\r\n", record_key);
		return FDS_ERR_NOT_FOUND;
	}

	fds_flash_record_t record;
	memset(&record, 0, sizeof(record));

	fds_record_open(&desc, &record);

	schedule->id = record_key;
	struct schedule_db_schema *schema = (struct schedule_db_schema*)record.p_data;
	schedule->day = schema->day;
	schedule->hour = schema->hour;
	schedule->minute = schema->minute;
	schedule->on = schema->on;

	NRF_LOG_INFO("schedule_read. record:%d day:%d hour:%d minute:%d on:%d\r\n",
			schedule->id, schedule->day, schedule->hour, schedule->minute, schedule->on);

	//TODO VALIDATION CHECK

	fds_record_close(&desc);

	return FDS_SUCCESS;
}

static void _load_schedule()
{
	uint8_t i;
	ret_code_t err_code;
	for (i=0; i<SCHEDULE_MAX_NR; i++) {
		struct schedule schedule;
		err_code = _schedule_read(INDEX_TO_RECORD_KEY(i), &schedule);
		if (err_code != FDS_SUCCESS) {
			schedule_desc.schedules[i].empty = true;
			continue;
		}

		schedule_desc.schedules[i].empty = false;
		memcpy(&schedule_desc.schedules[i].schedule, &schedule, sizeof(struct schedule)); 
	}

	schedule_desc.loaded = true;
}

static void _fds_evt_handler(fds_evt_t const * const p_evt)
{
	NRF_LOG_INFO("FDS EVT ID:%d\r\n", p_evt->id);
	switch (p_evt->id) {
		case FDS_EVT_INIT:
			_load_schedule();
			break;
		case FDS_EVT_WRITE:
			NRF_LOG_INFO("written file_id:%d record_key:%d\r\n", p_evt->write.file_id, p_evt->write.record_key);
			break;
		default:
			break;
	}
}

static ret_code_t _schedule_write(uint8_t record_key, struct schedule *schedule)
{
	db_schema.day = schedule->day;
	db_schema.hour = schedule->hour;
	db_schema.minute = schedule->minute;
	db_schema.on = schedule->on;

	fds_record_chunk_t chunk = {
		.p_data = (void*)&db_schema,
		.length_words = 1,
	};

	fds_record_t record_to_write = {
		.file_id = SCHEDULE_FILE_ID,
		.key = record_key,
		.data.p_chunks = &chunk,
		.data.num_chunks = 1,
	};

	fds_record_desc_t desc;
	memset(&desc, 0, sizeof(desc));

	ret_code_t err_code = fds_record_write(&desc, &record_to_write);
	if (err_code != 0) {
		NRF_LOG_ERROR("fds_record_write failed. %d\r\n", err_code);
	}

	return err_code;
}

uint8_t schedule_add(struct schedule *schedule)
{
	if (schedule->id == SCHDULE_ID_UNKNOWN) {
		int i;
		for (i=0; i<SCHEDULE_MAX_NR; i++)
			if (schedule_desc.schedules[i].empty)
				break;

		if (i == SCHEDULE_MAX_NR) {
			NRF_LOG_ERROR("schedule is full\r\n");
			return SCHDULE_ID_UNKNOWN;
		}

		schedule->id = i;
	}

	_schedule_write(INDEX_TO_RECORD_KEY(schedule->id), schedule);
	return schedule->id;
}

void schedule_init(void)
{
	schedule_desc.loaded = false;
	ret_code_t err_code = fds_register(_fds_evt_handler);

	APP_ERROR_CHECK(err_code);
}
