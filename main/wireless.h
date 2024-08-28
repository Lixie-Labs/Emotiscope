#define DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS (10 * (1000 * 60))	 // "10" is minutes
#define MAX_HTTP_REQUEST_ATTEMPTS (8)								 // Define the maximum number of retry attempts
#define INITIAL_BACKOFF_MS (1000)									 // Initial backoff delay in milliseconds
#define MAX_NETWORK_CONNECT_ATTEMPTS (3)

#define WEB_SERVER "emotiscope.rocks"
#define WEB_PORT "443"
#define WEB_URL "https://app.emotiscope.rocks/discovery/"

int64_t next_discovery_check_in_time = 0;

char mac_str[18]; // MAC address string format "XX:XX:XX:XX:XX:XX" + '\0'

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

bool esp_wifi_is_connected() {
    wifi_ap_record_t ap_info;
    return (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
}

static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST){
    static uint8_t attempt_count = 0;  // Keep track of the current attempt count

    char buf[512];
    int ret, len;

    esp_tls_t *tls = esp_tls_init();
    if (!tls) {
        ESP_LOGE(TAG, "Failed to allocate esp_tls handle!");
        return;
    }

    if (esp_tls_conn_http_new_sync(WEB_SERVER_URL, &cfg, tls) == 1) {
        ESP_LOGI(TAG, "Connection established...");
        attempt_count = 0; // Reset attempt count on success
    } else {
        ESP_LOGE(TAG, "Connection failed...");
        int esp_tls_code = 0, esp_tls_flags = 0;
        esp_tls_error_handle_t tls_e = NULL;
        esp_tls_get_error_handle(tls, &tls_e);
        /* Try to get TLS stack level error and certificate failure flags, if any */
        ret = esp_tls_get_and_clear_last_error(tls_e, &esp_tls_code, &esp_tls_flags);
        if (ret == ESP_OK) {
            ESP_LOGE(TAG, "TLS error = -0x%x, TLS flags = -0x%x", esp_tls_code, esp_tls_flags);
        }

        if (attempt_count < MAX_HTTP_REQUEST_ATTEMPTS) {
            uint32_t backoff_delay = INITIAL_BACKOFF_MS * (1 << attempt_count); // Calculate the backoff delay
            next_discovery_check_in_time = t_now_ms + backoff_delay; // Schedule the next attempt
            attempt_count++; // Increment the attempt count
            ESP_LOGE(TAG, "Retrying with backoff delay of %lums.", backoff_delay);
        } else {
            ESP_LOGE(TAG, "Couldn't reach server in time, will try again in a few minutes.");
            next_discovery_check_in_time = t_now_ms + DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS; // Reset to regular interval after max attempts
            attempt_count = 0; // Reset attempt count
        }

        goto cleanup;
    }

    size_t written_bytes = 0;
    while (written_bytes < strlen(REQUEST)) {
        ret = esp_tls_conn_write(tls, REQUEST + written_bytes, strlen(REQUEST) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            goto cleanup;
        }
    }

    ESP_LOGI(TAG, "Reading HTTP response...");
    while (1) {
        len = sizeof(buf) - 1;
        memset(buf, 0x00, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        } else if (ret < 0) {
            ESP_LOGE(TAG, "esp_tls_conn_read returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            break;
        } else if (ret == 0) {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGD(TAG, "%d bytes read", len);

        const char* check_in_success = "{\"check_in\":true}";

        char* trimmed_response = (buf + len - strlen(check_in_success)); // Get last N chars of buf, where N == strlen() of the response we seek
        ESP_LOGI(TAG, "TRIMMED RESPONSE: %s", trimmed_response);

        if (strcmp(trimmed_response, check_in_success) == 0) {
            next_discovery_check_in_time = t_now_ms + DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS; // Schedule the next check-in
            ESP_LOGI(TAG, "Checked in successfully!");
            break; // Exit the loop since we got a successful response
        } else {
            next_discovery_check_in_time = t_now_ms + 5000; // If server didn't respond correctly, try again in 5 seconds
            ESP_LOGE(TAG, "ERROR: BAD CHECK-IN RESPONSE");
            break; // Exit the loop to prevent hanging
        }
    }

cleanup:
    esp_tls_conn_destroy(tls);
}

void discovery_check_in(){
    static uint8_t attempt_count = 0;  // Keep track of the current attempt count
    int64_t t_now_ms = esp_log_timestamp(); // Get the current time in milliseconds

    if (t_now_ms >= next_discovery_check_in_time) {
		if(esp_wifi_is_connected() == true){
			ESP_LOGI(TAG, "checking into server...");
			next_discovery_check_in_time = t_now_ms + 3000; 

			esp_tls_cfg_t cfg = {
				.crt_bundle_attach = esp_crt_bundle_attach,
			};

			esp_netif_ip_info_t ip_info;
			esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
			esp_netif_get_ip_info(netif, &ip_info);

			char request_body[256];
			snprintf(request_body, sizeof(request_body), 
						"product=emotiscope&version=%d.%d.%d&nickname=%s&local_ip=" IPSTR,
						SOFTWARE_VERSION_MAJOR, 
						SOFTWARE_VERSION_MINOR, 
						SOFTWARE_VERSION_PATCH,
						device_nickname,
						IP2STR(&ip_info.ip));

			char check_in_request[1024];
			snprintf(check_in_request, sizeof(check_in_request),
					"POST " WEB_URL " HTTP/1.1\r\n"
					"Host: " WEB_SERVER "\r\n"
					"User-Agent: esp-idf/1.0 esp32\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Content-Length: %d\r\n"
					"\r\n"
					"%s",
					strlen(request_body), request_body);

			https_get_request(cfg, WEB_URL, check_in_request);

		}
		else{
			ESP_LOGE(TAG, "WiFi not connected before discovery server POST. Retrying in 5 seconds.");
			next_discovery_check_in_time = t_now_ms + 5000;	 // Retry in 5 seconds if WiFi is not connected
		}
	}
}

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
	//ESP_LOGI(TAG, "WSRX: %s", ws_pkt.payload);

	parse_emotiscope_packet(req);

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
	ESP_LOGI(TAG, "stop_websocket_server()");
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

			set_improv_error_state(IMPROV_ERROR_UNABLE_TO_CONNNECT);
			improv_current_state = IMPROV_CURRENT_STATE_READY;

			esp_wifi_connect();  // Reconnect on disconnect
			connection_attempts++;
		}
    }
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        uart_print("GOT IP ADDRESS\n");

		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

		ESP_LOGI(TAG, "IP Address: %s", ip_str);

		esp_netif_ip_info_t ip_info;
		esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
		esp_netif_get_ip_info(netif, &ip_info);

		//esp_netif_dns_info_t dns_info;
		//dns_info.ip.u_addr.ip4.addr = inet_addr("8.8.8.8");  // Google's public DNS server
		//esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns_info);

		improv_current_state = IMPROV_CURRENT_STATE_PROVISIONED;
		set_improv_error_state(IMPROV_ERROR_NONE);
		send_current_state();
		send_redirect_url(0x01);
		
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
		ESP_LOGE(TAG, "No WiFi credentials found in NVS!");
	}

    // Disable power save mode
	//ESP_LOGI(TAG, "Disabling power save mode");
    esp_wifi_set_ps(WIFI_PS_NONE);
}