#include <stdlib.h>
#include <string.h>
#include "app_error.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "ble_gatts.h"
#include "now.h"

#define NRF_LOG_MODULE_NAME "MS"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define MOTOR_CHAR_UUID		0x0000
#define MOTOR_SERVICE_UUID	0xf000
#define MOTOR_MOVE_CMD		1

static void (*_motor_move)(void);
static uint16_t _service_handle;
static ble_gatts_char_handles_t _char_handle;
static uint16_t _conn_handle = BLE_CONN_HANDLE_INVALID;

static uint32_t _gatt_char_init(void)
{
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
	char_md.char_props.write = 1;

    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
	attr_md.write_perm.sm = 1;
	attr_md.write_perm.lv = 1;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;

	ble_uuid_t ble_uuid;
    BLE_UUID_BLE_ASSIGN(ble_uuid, MOTOR_SERVICE_UUID);
    ble_gatts_attr_t attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));

	uint8_t initial_value[1] = {0};
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = initial_value;

    return sd_ble_gatts_characteristic_add(_service_handle, &char_md, &attr_char_value, &_char_handle);
}

static uint32_t _gatt_service_add(void)
{
	ble_uuid_t uuid;
	BLE_UUID_BLE_ASSIGN(uuid, MOTOR_SERVICE_UUID);

	return sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &_service_handle);
}

void motor_service_evt_handler(ble_evt_t *p_ble_evt)
{
	//uint32_t err_code;
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
				if (!_motor_move || _char_handle.value_handle != p_evt_write->handle ||
						p_evt_write->len != sizeof(uint8_t) || p_evt_write->data[0] != MOTOR_MOVE_CMD)
					return;

				NRF_LOG_INFO("Move MOTOR(%x)\r\n", p_evt_write->data[0]);
				_motor_move();
			}
			break;
	}
}

uint32_t motor_service_init(void (*motor_move)(void))
{
	uint32_t error_code = _gatt_service_add();
	if (error_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("_gatt_service_add failed\r\n");
        return error_code;
	}

	error_code = _gatt_char_init();
    if (error_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("_gatt_char_init failed\r\n");
    }

	_motor_move = motor_move;

	return error_code;
}
