#include <stdint.h>
#include <string.h>
#include "app_error.h"

#include "ble_dis.h"

void device_information_service_init(const char* manufacturer_name)
{
	uint32_t err_code;
	ble_dis_init_t dis_init;

	// Initialize Device Information Service.
	memset(&dis_init, 0, sizeof(dis_init));

	ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)manufacturer_name);

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);
}
