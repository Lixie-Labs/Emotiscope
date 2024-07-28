#define ESP_ERROR_PRINT(x) do {                                         \
    esp_err_t __err_rc = (x);                                           \
    if (__err_rc != ESP_OK) {                                           \
        ESP_LOGE(TAG, "ERROR: esp_err_t 0x%x", __err_rc); \
    }                                                                   \
} while(0)

extern void parse_emotiscope_packet(httpd_req_t* req);

// Websocket server
static httpd_handle_t server = NULL;

// Event group for WiFi events
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Buffer for WS packets
char websocket_packet_buffer[1024];

bool connected_to_wifi = false;

// Transmit a WS packet
esp_err_t wstx(httpd_req_t* req, char* data){
    httpd_ws_frame_t ws_pkt;
    dsps_memset_aes3(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	ws_pkt.payload = (uint8_t*)data;
	ws_pkt.len = strlen(data)+1;
    
	ESP_LOGI(TAG, "WSTX: %s", ws_pkt.payload);

	esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }

	return ret;
}

// Called whenever a WS packet is received
esp_err_t wsrx(httpd_req_t* req){
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    dsps_memset_aes3(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	dsps_memset_aes3(websocket_packet_buffer, 0, 1024);
    
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	ws_pkt.payload = (uint8_t*)websocket_packet_buffer;
    
	httpd_ws_recv_frame(req, &ws_pkt, 1024);
	ESP_LOGI(TAG, "WSRX: %s", ws_pkt.payload);

	parse_emotiscope_packet(req);

    return ESP_OK;
}

esp_err_t wstx_broadcast(const char *message) {
	//ESP_LOGI(TAG, "wstx_broadcast(\"%s\")", message);
    size_t clients;

    int client_fds[CONFIG_LWIP_MAX_ACTIVE_TCP];
    
    esp_err_t ret = httpd_get_client_list(server, &clients, client_fds);
    if (ret != ESP_OK) {
		ESP_LOGE(TAG, "httpd_get_client_list failed with %d", ret);
        return ret;
    }

	ESP_LOGI(TAG, "Num clients: %zu", clients);

	if (clients == 0) {
		ESP_LOGI(TAG, "No clients connected");
		return ESP_OK;
	}

    for (size_t i = 0; i < clients; i++) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t*)message;
        ws_pkt.len = strlen(message);
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        httpd_ws_send_frame(server, &ws_pkt);
    }

    return ESP_OK;
}

// Websocket handler URI
static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = wsrx,
        .user_ctx   = NULL,
        .is_websocket = true
};

// Start the websocket server
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

// Called whenever a WiFi event occurs, like connecting or disconnecting
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	// Starting connection
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connecting to %s", wifi_ssid);
        esp_wifi_connect();
    }
	
	// Lost connection
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGE(TAG, "Disconnected from %s, attempting to reconnect", wifi_ssid);
        esp_wifi_connect();
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		connected_to_wifi = false;
    }
	
	// Sucessfully connected
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		connected_to_wifi = true;

		ESP_LOGI(TAG, "Starting websocket server");
		server = start_websocket_server();
	}
}

// Save SSID and PASS to NVS
void save_wifi_credentials(char* w_ssid, char* w_pass){
	put_string("wifi_ssid", w_ssid);
	put_string("wifi_pass", w_pass);
	
	ESP_LOGI(TAG, "WiFi credentials saved as SSID: %s, PASS: %s", w_ssid, w_pass);
}

// Load SSID and PASS from NVS
bool load_wifi_credentials(){
	ESP_LOGI(TAG, "load_wifi_credentials()");

	// Clear SSID and PASS ----------------------------------------------------------
	dsps_memset_aes3(wifi_ssid, 0, 128);
	dsps_memset_aes3(wifi_pass, 0, 128);

	// Load SSID --------------------------------------------------------------------
	size_t ssid_len = get_string("wifi_ssid", wifi_ssid, 128);
	if(ssid_len == 0){
		ESP_LOGE(TAG, "No SSID found in NVS yet!");
		return false;
	}

	// Load PASS --------------------------------------------------------------------
	size_t pass_len = get_string("wifi_pass", wifi_pass, 128);
	if(pass_len == 0){
		ESP_LOGE(TAG, "No PASS found in NVS yet!");
		return false;
	}

	//ESP_LOGI(TAG, "WiFi credentials loaded as SSID: %s, PASS: %s", wifi_ssid, wifi_pass);
	return true;
}

// Set up WIFI_STA mode and connect to network
void connect_to_network(){
	ESP_LOGI(TAG, "connect_to_network()");
	
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
        ESP_LOGI(TAG, "connected to SSID: %s", wifi_ssid);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", wifi_ssid);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

// WiFi begins here
void init_wifi(){
	ESP_LOGI(TAG, "init_wifi()");

	// Needed on first run
	save_wifi_credentials("WilsonAve", "Westies1!");

	if( load_wifi_credentials() == true ){
		connect_to_network();
	}
	else{
		ESP_LOGE(TAG, "No WiFi credentials found in NVS!");
	}

    // Disable power save mode
	ESP_LOGI(TAG, "Disabling power save mode");
    esp_wifi_set_ps(WIFI_PS_NONE);
}