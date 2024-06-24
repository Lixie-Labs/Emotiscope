#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "freertos/event_groups.h"

#define ESP_ERROR_PRINT(x) do {                                         \
    esp_err_t __err_rc = (x);                                           \
    if (__err_rc != ESP_OK) {                                           \
        ESP_LOGE(TAG, "ESP_ERROR_PRINT failed: esp_err_t 0x%x", __err_rc); \
    }                                                                   \
} while(0)

char wifi_ssid[128];
char wifi_pass[128];

static httpd_handle_t server = NULL;
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static esp_err_t ws_handler(httpd_req_t *req){
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);

    ws_pkt.payload[0] = 'B';
    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    return ret;
}

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static httpd_handle_t start_websocket_server(void){
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connecting to the AP");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected from AP");
        esp_wifi_connect();
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_websocket_server();
    }
}

static void wifi_disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (httpd_stop(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

void wifi_init_sta(const char* wifi_ssid, const char* wifi_pass) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, wifi_pass, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_LOGI(TAG, "Connecting to %s", wifi_ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", wifi_ssid);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", wifi_ssid);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void save_wifi_credentials(char* w_ssid, char* w_pass){
	esp_err_t err;

	// SET SSID --------------------------------------------------------------------
    err = nvs_set_blob(config_handle, "wifi_ssid", w_ssid, strlen(w_ssid) + 1);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error (%s) saving wifi_ssid to NVS", esp_err_to_name(err));
	}

	// SET PASS --------------------------------------------------------------------
    err = nvs_set_blob(config_handle, "wifi_pass", w_pass, strlen(w_pass) + 1);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error (%s) saving wifi_pass to NVS", esp_err_to_name(err));
	}

    // Commit
    err = nvs_commit(config_handle);
    if (err != ESP_OK){
		ESP_LOGE(TAG, "Error (%s) committing wifi_ssid to NVS", esp_err_to_name(err));
	}

	ESP_LOGI(TAG, "WiFi credentials saved as SSID: %s, PASS: %s", w_ssid, w_pass);
}

void load_wifi_credentials(){
	ESP_LOGI(TAG, "load_wifi_credentials()");

	// READ SSID ----------------------------------------------------------------
    // Read the size of memory space required for blob
    size_t ssid_required_size = 0;  // value will default to 0, if not set yet in NVS
    esp_err_t err = nvs_get_blob(config_handle, "wifi_ssid", NULL, &ssid_required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
		ESP_LOGE(TAG, "Error (%s) reading wifi_ssid size from NVS", esp_err_to_name(err));
	}

	ESP_LOGI(TAG, "SSID size: %d", ssid_required_size);

    // Read previously saved blob if available
    char* w_ssid = malloc(ssid_required_size + 1);
    if (ssid_required_size > 0) {
        err = nvs_get_blob(config_handle, "wifi_ssid", w_ssid, &ssid_required_size);
        if (err != ESP_OK) {
            free(w_ssid);
            ESP_LOGE(TAG, "Error (%s) reading wifi_ssid from NVS", esp_err_to_name(err));
        }
    }

	// READ PASS ----------------------------------------------------------------
    // Read the size of memory space required for blob
    size_t pass_required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_get_blob(config_handle, "wifi_pass", NULL, &pass_required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
		ESP_LOGE(TAG, "Error (%s) reading wifi_pass size from NVS", esp_err_to_name(err));
	}

    // Read previously saved blob if available
    char* w_pass = malloc(pass_required_size + 1);
    if (pass_required_size > 0) {
        err = nvs_get_blob(config_handle, "wifi_pass", w_pass, &pass_required_size);
        if (err != ESP_OK) {
            free(w_pass);
            ESP_LOGE(TAG, "Error (%s) reading wifi_pass from NVS", esp_err_to_name(err));
        }
    }

	memset(wifi_ssid, 0, sizeof(wifi_ssid));
	memset(wifi_pass, 0, sizeof(wifi_pass));
	strncpy(wifi_ssid, w_ssid, sizeof(wifi_ssid));
	strncpy(wifi_pass, w_pass, sizeof(wifi_pass));

	free(w_ssid);
	free(w_pass);

	ESP_LOGI(TAG, "WiFi credentials loaded as SSID: %s, PASS: %s", wifi_ssid, wifi_pass);
}

void init_wifi(){
	ESP_LOGI(TAG, "init_wifi()");

    // Disable power save mode
    esp_wifi_set_ps(WIFI_PS_NONE);

	// Save wifi credentials
	//save_wifi_credentials("testnet", "testpass");

	// Load wifi credentials
	load_wifi_credentials();

	ESP_LOGI(TAG, "WiFi SSID: %s", wifi_ssid);

	ESP_LOGI(TAG, "Attempting connection to %s...", wifi_ssid);
	// Connect to network
    wifi_init_sta(wifi_ssid, wifi_pass);

	// Register event handlers
    ESP_ERROR_PRINT(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_connect_handler, &server));
    ESP_ERROR_PRINT(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_disconnect_handler, &server));

	// Start webserver
    server = start_websocket_server();
}