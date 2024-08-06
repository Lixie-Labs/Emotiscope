extern void parse_emotiscope_packet(httpd_req_t* req);

// Websocket server
httpd_handle_t server = NULL;

// Event group for WiFi events
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Buffer for WS packets
char websocket_packet_buffer[1024];

bool connected_to_wifi = false;

struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

// Transmit a WS packet
esp_err_t wstx(httpd_req_t* req, char* data){
    httpd_ws_frame_t ws_pkt;
    dsps_memset_aes3(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	ws_pkt.payload = (uint8_t*)data;
	ws_pkt.len = strlen(data)+1;
    
	//ESP_LOGI(TAG, "WSTX: %s", ws_pkt.payload);

	esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        //ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
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

	// Echo back
	/*
	esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
	*/

    return ESP_OK;
}

esp_err_t wstx_broadcast(const char *message) {
	/*
	//ESP_LOGI(TAG, "wstx_broadcast(\"%s\")", message);
    size_t clients;

    int client_fds[CONFIG_LWIP_MAX_ACTIVE_TCP];
    
    esp_err_t ret = httpd_get_client_list(server, &clients, &client_fds);
    if (ret != ESP_OK) {
		//ESP_LOGE(TAG, "httpd_get_client_list failed with %x", ret);
        return ret;
    }

	ESP_LOGI(TAG, "Num clients: %zu", clients);

	if (clients == 0) {
		//ESP_LOGI(TAG, "No clients connected");
		return ESP_OK;
	}

    for (size_t i = 0; i < clients; i++) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t*)message;
        ws_pkt.len = strlen(message);
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

		ret = httpd_ws_send_frame_async(server, client_fds[i], &ws_pkt);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
		}
    }
	*/

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
void start_websocket_server(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
		ESP_LOGI(TAG, "Server started");

        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        if(httpd_register_uri_handler(server, &ws) == ESP_OK){
			ESP_LOGI(TAG, "URI handler registered");
		}
		else{
			ESP_LOGE(TAG, "Error registering URI handler");
		}
		
    }
	else{
		ESP_LOGE(TAG, "Error starting server!");
	}
}

void stop_websocket_server(void){
	httpd_stop(server);
}

// Save SSID and PASS to NVS
void save_wifi_credentials(char* w_ssid, char* w_pass){
	put_string("wifi_ssid", w_ssid);
	put_string("wifi_pass", w_pass);

	memcpy(wifi_ssid, w_ssid, strlen(w_ssid)+1);
	memcpy(wifi_pass, w_pass, strlen(w_pass)+1);
	
	//ESP_LOGI(TAG, "WiFi credentials saved as SSID: %s, PASS: %s", w_ssid, w_pass);
}

// Load SSID and PASS from NVS
bool load_wifi_credentials(){
	//ESP_LOGI(TAG, "load_wifi_credentials()");

	// Clear SSID and PASS ----------------------------------------------------------
	dsps_memset_aes3(wifi_ssid, 0, 128);
	dsps_memset_aes3(wifi_pass, 0, 128);

	// Load SSID --------------------------------------------------------------------
	size_t ssid_len = get_string("wifi_ssid", wifi_ssid, 128);
	if(ssid_len == 0){
		//ESP_LOGE(TAG, "No SSID found in NVS yet!");
		return false;
	}

	// Load PASS --------------------------------------------------------------------
	size_t pass_len = get_string("wifi_pass", wifi_pass, 128);
	if(pass_len == 0){
		//ESP_LOGE(TAG, "No PASS found in NVS yet!");
		return false;
	}

	////ESP_LOGI(TAG, "WiFi credentials loaded as SSID: %s, PASS: %s", wifi_ssid, wifi_pass);
	return true;
}







static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	static uint8_t connection_attempts = 1;

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
		uart_print("CONNECTED TO NETWORK: ");		
		uart_print(wifi_ssid);
		uart_printc('\n');
		connection_attempts = 1;
	}
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if(connection_attempts <= 5){
			uart_print("COULDN'T REACH NETWORK: ");
			uart_print(wifi_ssid);
			uart_print(" - attempt ");
			uart_printd(connection_attempts);
			uart_printc('\n');

			connected_to_wifi = false;

			send_error_state(IMPROV_ERROR_UNABLE_TO_CONNNECT);

			improv_current_state = IMPROV_CURRENT_STATE_READY;
			send_current_state();

			esp_wifi_connect();  // Reconnect on disconnect
			connection_attempts++;
		}
    }
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        uart_print("GOT IP ADDRESS\n");
		//usb_print("GOT IP ADDRESS\n");

		char ip_address[16];
		// convert IP to string

		improv_current_state = IMPROV_CURRENT_STATE_PROVISIONED;
		send_current_state();
		
		connected_to_wifi = true;

		ESP_LOGI(TAG, "Starting websocket server");
		start_websocket_server();

		connection_attempts = 1;
    }
}

// Function to initialize and start the Wi-Fi connection
void connect_wifi() {
    static bool wifi_initialized = false;
    wifi_config_t wifi_config = {};

    // Initialize the TCP/IP stack if not already done
    if (!wifi_initialized) {
        esp_netif_init();
        esp_event_loop_create_default();
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        wifi_initialized = true;
    }

    // Configure and start the Wi-Fi driver
    strncpy((char*)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, wifi_pass, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

	uart_print("Connecting to SSID: ");
	uart_print(wifi_ssid);
}

// WiFi begins here
void init_wifi(){
	//ESP_LOGI(TAG, "init_wifi()");

	if( load_wifi_credentials() == true ){
		connect_wifi();
	}
	else{
		//ESP_LOGE(TAG, "No WiFi credentials found in NVS!");
	}

    // Disable power save mode
	//ESP_LOGI(TAG, "Disabling power save mode");
    esp_wifi_set_ps(WIFI_PS_NONE);
}