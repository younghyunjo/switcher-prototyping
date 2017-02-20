#include <stdint.h>
#include <string.h>
#include "app_error.h"
#include "ble_bas.h"
#include "ble_gap.h"

static ble_bas_t m_bas;
static void (*_cb_connect)(void) = NULL;

#if 0
void battery_service_db_disc_evt(ble_db_discovery_evt_t * p_evt)
{
    ble_bas_on_db_disc_evt(&m_ble_bas_c, p_evt);
}
#endif

void battery_service_evt_handler(ble_evt_t *p_ble_evt)
{
	if (p_ble_evt->header.evt_id == BLE_GAP_EVT_CONNECTED && _cb_connect) {
		_cb_connect();
	}

	ble_bas_on_ble_evt(&m_bas, p_ble_evt);
}

void battery_service_level_update(uint8_t level)
{
    uint32_t err_code = ble_bas_battery_level_update(&m_bas, level);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_PACKETS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}

void battery_service_init(void (*cb_connect)(void)) 
{
    uint32_t err_code;
    ble_bas_init_t bas_init;
    memset(&bas_init, 0, sizeof(bas_init));
	_cb_connect = cb_connect;

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = false;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);
}
