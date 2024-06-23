#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_log.h"
//#include "protocol_examples_common.h"
#include <esp_http_server.h>

#include "wireless.h"

void app_main(void){
	// Init NVS flash
	ESP_ERROR_CHECK(nvs_flash_init());

	init_wifi();
}
