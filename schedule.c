#include <stdint.h>
#include <stdbool.h>

#include "app_error.h"
#include "fds.h"
#include "schedule.h"

#define NRF_LOG_MODULE_NAME "SCH"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define SCHEDULE_FILE_ID			0x01
#define SCHEDULE_RECORD_KEY_START	0x00// TODO DELETE ME
#define SCHEDULE_MAX_NR 			10

struct schedule_record {
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t on;
};

struct schedule_desc {
	struct schedule_record record;
	uint8_t record_key;
	bool empty;
};

static struct schedule_desc schedules[SCHEDULE_MAX_NR];
static uint8_t record_chunk[sizeof(struct schedule_desc)] __ALIGN(4) ;

static void _load_schedule(void)
{
	uint8_t i;
	ret_code_t err_code;

    fds_record_desc_t desc;
    fds_find_token_t ft;
    fds_flash_record_t record;

	for (i=0; i<SCHEDULE_MAX_NR; i++) {
		memset(&desc, 0, sizeof(desc));
		memset(&ft, 0, sizeof(ft));

		err_code = fds_record_find(SCHEDULE_FILE_ID, i+1, &desc, &ft);
		if (err_code != FDS_SUCCESS) {
			memset(&schedules[i].record, 0xff, sizeof(struct schedule_desc));
			schedules[i].record_key = i+1;
			schedules[i].empty = true;
			NRF_LOG_INFO("%d record empty\r\n", i+1);
			continue;
		}

		memset(&record, 0, sizeof(record));

		fds_record_open(&desc, &record);
		memcpy(&schedules[i].record, record.p_data, sizeof(struct schedule_desc));
		schedules[i].empty = false;

		NRF_LOG_INFO("day:%d hour:%d minute:%d on:%d\r\n",
				schedules[i].record.day,
				schedules[i].record.hour,
				schedules[i].record.minute,
				schedules[i].record.on);

		//TODO VALIDATION CHECK
		fds_record_close(&desc);
	}
}

static uint8_t chunk[sizeof(struct schedule_desc)] __ALIGN(4) ;


void write_test(void)
{
	chunk[0] = 0xaa;
	chunk[1] = 0x00;
	chunk[2] = 0xff;
	chunk[3] = 0xdd;

    fds_record_desc_t desc = {0};
	fds_record_chunk_t c = {
		.p_data = chunk,
		.length_words = 1
	};

	fds_record_t record_to_write = {
		.file_id = SCHEDULE_FILE_ID,
		.key = SCHEDULE_RECORD_KEY_START,
		.data.p_chunks = &c,
		.data.num_chunks = 1,
	};

	ret_code_t err_code = fds_record_write(&desc, &record_to_write);
	NRF_LOG_INFO("fds_record_write :%d\r\n", err_code);
}

void read_test(void)
{
	ret_code_t err_code;
    fds_record_desc_t desc;

	//memset(chunk, 0, sizeof(chunk));

    fds_find_token_t ft;
	memset(&ft, 0, sizeof(fds_find_token_t));

	NRF_LOG_INFO("BEFORE\r\n");
	err_code = fds_record_find(SCHEDULE_FILE_ID, SCHEDULE_RECORD_KEY_START, &desc, &ft);
	NRF_LOG_INFO("fds_record_find :%d\r\n", err_code);

    fds_flash_record_t record          = {0};
	err_code = fds_record_open(&desc, &record);
	NRF_LOG_INFO("%x\r\n", ((char*)record.p_data)[0]);
	fds_record_close(&desc);
	APP_ERROR_CHECK(err_code);
}

void sch_test(void)
{
	write_test();
	read_test();
}

static void _fds_evt_handler(fds_evt_t const * const p_evt)
{
	NRF_LOG_INFO("FDS EVT ID:%d\r\n", p_evt->id);
	switch (p_evt->id) {
		case FDS_EVT_INIT:
			_load_schedule();
			break;
		default:
			break;
	}
}

void schedule_add(struct schedule_info *info)
{
	int i;
	for (i=0; i<SCHEDULE_MAX_NR; i++) {
		if (!schedules[i].empty) {
			continue;
		}

		schedules[i].record.day = info->day;
		schedules[i].record.hour = info->hour;
		schedules[i].record.minute = info->minute;
		schedules[i].record.on = info->on;
		schedules[i].record_key = i+1;
		schedules[i].empty = false;

		memcpy(record_chunk, &schedules[i].record, sizeof(struct schedule_record));

		fds_record_chunk_t chunk = {
			.p_data = record_chunk,
			.length_words = 1,
		};

		fds_record_t record_to_write = {
			.file_id = SCHEDULE_FILE_ID,
			.key = schedules[i].record_key,
			.data.p_chunks = &chunk,
			.data.num_chunks = 1,
		};

		fds_record_desc_t desc;
		memset(&desc, 0, sizeof(desc));

		ret_code_t err_code = fds_record_write(&desc, &record_to_write);
		if (err_code != 0) {
			NRF_LOG_ERROR("fds_record_write failed. %d\r\n", err_code);
		}
		else {
			NRF_LOG_INFO("key:%d added\r\n", schedules[i].record_key);
		}

		break;
	}
}

void schedule_init(void)
{
	memset(schedules, 0xff, sizeof(schedules));

	ret_code_t err_code = fds_register(_fds_evt_handler);

	APP_ERROR_CHECK(err_code);
}
