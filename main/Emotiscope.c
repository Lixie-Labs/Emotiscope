#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_system.h>
#include "nvs_flash.h"
#include "nvs.h"
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_log.h"
#include <esp_http_server.h>

#define TAG "Emotiscope"

#include "types.h"
#include "configuration.h"
#include "wireless.h"
#include "system.h"

void app_main(void){
	init_system();
}