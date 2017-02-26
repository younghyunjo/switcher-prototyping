#include <stdlib.h>
#include <string.h>
#include "app_error.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "ble_gatts.h"
#include "now.h"
#include "schedule.h"

#define NRF_LOG_MODULE_NAME "SCS"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define SCHEDULE_LIST_CHAR_UUID		0xfa02
#define SCHEDULE_SERVICE_UUID		0xfa01

static uint16_t _service_handle;
static ble_gatts_char_handles_t _char_handle;
static uint16_t _conn_handle = BLE_CONN_HANDLE_INVALID;

static void _schedule_list_update(void)
{
	struct schedule schedules[SCHEDULE_MAX_NR];
	schedule_list(schedules);

	ble_gatts_value_t gatts_value;
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = sizeof(schedules);
	gatts_value.offset  = 0;
	gatts_value.p_value = (uint8_t*)schedules;

	uint32_t err_code = sd_ble_gatts_value_set(_conn_handle, _char_handle.value_handle, &gatts_value);
	APP_ERROR_CHECK(err_code);

}

static uint32_t _gatt_char_init(void)
{
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
	char_md.char_props.read = 1;

    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
	attr_md.read_perm.sm = 1;
	attr_md.read_perm.lv = 1;
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 1;

	ble_uuid_t ble_uuid;
    BLE_UUID_BLE_ASSIGN(ble_uuid, SCHEDULE_LIST_CHAR_UUID);
    ble_gatts_attr_t attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));

	//uint8_t initial_value[1] = {0};
	struct schedule schdule[SCHEDULE_MAX_NR];
	memset(schdule, 0xff, sizeof(schdule));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(struct schedule) * SCHEDULE_MAX_NR;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(struct schedule) * SCHEDULE_MAX_NR;
    attr_char_value.p_value   = (uint8_t*)schdule;

    return sd_ble_gatts_characteristic_add(_service_handle, &char_md, &attr_char_value, &_char_handle);
}

static uint32_t _gatt_service_add(void)
{
    ble_uuid_t ble_uuid;
    BLE_UUID_BLE_ASSIGN(ble_uuid, SCHEDULE_SERVICE_UUID);

    return sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &_service_handle);
}

void schedule_service_evt_handler(ble_evt_t *p_ble_evt)
{
	uint32_t err_code;

	//uint32_t err_code;
	switch(p_ble_evt->header.evt_id) 
	{
		case BLE_GAP_EVT_CONNECTED:
			_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			break;
		case BLE_GAP_EVT_DISCONNECTED:
			_conn_handle = BLE_CONN_HANDLE_INVALID;
			break;
		case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			{
				ble_gatts_evt_rw_authorize_request_t req = 
					p_ble_evt->evt.gatts_evt.params.authorize_request;

				if (req.type != BLE_GATTS_AUTHORIZE_TYPE_READ) {
					return;
				}

				if (req.request.read.uuid.uuid != SCHEDULE_LIST_CHAR_UUID) {
					return;
				}

				_schedule_list_update();

				ble_gatts_rw_authorize_reply_params_t auth_reply;
				memset(&auth_reply, 0, sizeof(auth_reply));
				auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
				auth_reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
				err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
						&auth_reply);
				APP_ERROR_CHECK(err_code);
			}
			break;
		case BLE_GATTS_EVT_WRITE:
			/*
			{
				ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
				if (!_motor_move || _char_handle.value_handle != p_evt_write->handle ||
						p_evt_write->len != sizeof(uint8_t) || p_evt_write->data[0] != MOTOR_MOVE_CMD)
					return;

				NRF_LOG_INFO("Move MOTOR(%x)\r\n", p_evt_write->data[0]);
				_motor_move();
			}
			*/
			break;
	}
}

uint32_t schedule_service_init(void)
{
	uint32_t err_code = _gatt_service_add();
    if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("_gatt_service_add failed\r\n");
        return err_code;
    }

	err_code = _gatt_char_init();
    if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("_gatt_char_init failed. %d\r\n", err_code);
    }

	NRF_LOG_INFO("init\r\n");
	return err_code;
}
