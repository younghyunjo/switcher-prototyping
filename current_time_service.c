#include <stdlib.h>
#include <string.h>
#include "app_error.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "ble_gatts.h"
#include "now.h"

#define NRF_LOG_MODULE_NAME "CTS"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

static uint16_t _conn_handle = BLE_CONN_HANDLE_INVALID;
static uint16_t service_handle;
static ble_gatts_char_handles_t current_time_handles;
static time_t (*_current_time_get)(void) = NULL;
static void (*_current_time_set)(time_t current_time) = NULL;

static time_t _array2time(uint8_t time_array[4])
{
	return (time_array[0] << 24) | (time_array[1] << 16) | (time_array[2] << 8) | (time_array[3]);
}

static void _time2array(uint8_t current_time[4])
{
	memset(current_time, 0, sizeof(current_time));
	if (!_current_time_get) {
		return;
	}

	time_t _current_time = _current_time_get();
	current_time[0] = _current_time >> 24;
	current_time[1] = _current_time >> 16;
	current_time[2] = _current_time >> 8;
	current_time[3] = _current_time;
}

static void _current_time_service_current_time_gatt_set(void)
{
	ble_gatts_value_t gatts_value;
	memset(&gatts_value, 0, sizeof(gatts_value));

	uint8_t current_time[4] = {0,};
	_time2array(current_time);

	gatts_value.len     = sizeof(time_t);
	gatts_value.offset  = 0;
	gatts_value.p_value = current_time;

	uint32_t err_code = sd_ble_gatts_value_set(_conn_handle, current_time_handles.value_handle, &gatts_value);
	APP_ERROR_CHECK(err_code);
}


void current_time_service_evt_handler(ble_evt_t *p_ble_evt)
{
	uint32_t err_code;

	switch(p_ble_evt->header.evt_id) 
	{
		case BLE_GAP_EVT_CONNECTED:
			_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			break;
		case BLE_GAP_EVT_DISCONNECTED:
			_conn_handle = BLE_CONN_HANDLE_INVALID;
			break;
		case BLE_GATTS_EVT_WRITE:
			{
				ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
				if (!_current_time_set || 
						current_time_handles.value_handle != p_evt_write->handle || 
						p_evt_write->len != sizeof(time_t)) {
					return;
				}

				_current_time_set(_array2time(p_evt_write->data));
			}
			break;
		case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			{
				ble_gatts_evt_rw_authorize_request_t req = 
					p_ble_evt->evt.gatts_evt.params.authorize_request;

				if (req.request.read.uuid.uuid != BLE_UUID_CURRENT_TIME_CHAR) {
					return;
				}
				if (req.type != BLE_GATTS_AUTHORIZE_TYPE_READ) {
					return;
				}

				_current_time_service_current_time_gatt_set();

				ble_gatts_rw_authorize_reply_params_t auth_reply;
				memset(&auth_reply, 0, sizeof(auth_reply));
				auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
				auth_reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
				err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
						&auth_reply);
				APP_ERROR_CHECK(err_code);
			}
			break;
	}
}

static uint32_t _gatt_char_init(void)
{
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
	char_md.char_props.read = 1;
	char_md.char_props.write = 1;

    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
	attr_md.read_perm.sm = 1;
	attr_md.read_perm.lv = 1;
	attr_md.write_perm.sm = 1;
	attr_md.write_perm.lv = 1;
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 1;

	ble_uuid_t ble_uuid;
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_CURRENT_TIME_CHAR);
    ble_gatts_attr_t attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));

	uint8_t initial_time[4] = {0};
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 4;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 4;
    attr_char_value.p_value   = initial_time;

    return sd_ble_gatts_characteristic_add(service_handle, &char_md, &attr_char_value, &current_time_handles);
}

static uint32_t _gatt_service_add(void)
{
    ble_uuid_t ble_uuid;
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_CURRENT_TIME_SERVICE);

    return sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &service_handle);
}

uint32_t current_time_service_init(time_t (*current_time_get)(void), void (*current_time_set)(time_t current_time))
{
	uint32_t err_code = _gatt_service_add();
    if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("_gatt_service_add failed\r\n");
        return err_code;
    }

	err_code = _gatt_char_init();
    if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("_gatt_char_init failed\r\n");
    }

	_current_time_get = current_time_get;
	_current_time_set = current_time_set;

	return err_code;
}
