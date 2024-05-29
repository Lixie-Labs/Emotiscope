#define DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS (10 * (1000 * 60))	 // "10" is minutes
#define MAX_HTTP_REQUEST_ATTEMPTS (8)								 // Define the maximum number of retry attempts
#define INITIAL_BACKOFF_MS (1000)									 // Initial backoff delay in milliseconds
#define MAX_NETWORK_CONNECT_ATTEMPTS (3)

#define DISCOVERY_SERVER_URL "https://app.emotiscope.rocks/discovery/"

String WEB_VERSION = "";

DNSServer dns_server; // DNS server instance

// Define a char array to hold the formatted MAC address string
char mac_str[18]; // MAC address string format "XX:XX:XX:XX:XX:XX" + '\0'

PsychicHttpServer server;
PsychicWebSocketHandler websocket_handler;

volatile bool web_server_ready = false;
int16_t connection_status = -1;

uint8_t network_connection_attempts = 0;

void reboot_into_wifi_config_mode() {
	preferences.putBool("CONFIG_TRIG", true);
	delay(100);
	ESP.restart();
}

void discovery_check_in() {
	static uint32_t next_discovery_check_in_time = 0;
	static uint8_t attempt_count = 0;  // Keep track of the current attempt count
	//uint32_t t_now_ms = millis();

	if (t_now_ms >= next_discovery_check_in_time) {
		// Check Wi-Fi connection status
		if (WiFi.status() == WL_CONNECTED) {
			HTTPClient http_client;
			http_client.begin(DISCOVERY_SERVER_URL);
			http_client.addHeader("Content-Type", "application/x-www-form-urlencoded");

			char params[120];
			snprintf(params, 120, "product=emotiscope&version=%d.%d.%d&local_ip=%s", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH, WiFi.localIP().toString().c_str());

			int http_response_code = http_client.POST(params);	// Make the request

			if (http_response_code == 200) {						// Check for a successful response
				printf("RESPONSE CODE: %i\n", http_response_code);	// Print HTTP return code
				String response = http_client.getString();			// Get the request response payload
				printf("RESPONSE BODY: %s\n", response.c_str());	// Print request response payload

				if (response.equals("{\"check_in\":true}")) {
					next_discovery_check_in_time = t_now_ms + DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS;	 // Schedule the next check-in
					printf("Check in successful!\n");
				}
				else {
					next_discovery_check_in_time = t_now_ms + 5000;	 // If server didn't respond correctly, try again in 5 seconds
					printf("ERROR: BAD CHECK-IN RESPONSE\n");
				}
				attempt_count = 0;	// Reset attempt count on success
			}
			else {
				printf("Error on sending POST: %d\n", http_response_code);
				if (attempt_count < MAX_HTTP_REQUEST_ATTEMPTS) {
					uint32_t backoff_delay = INITIAL_BACKOFF_MS * (1 << attempt_count);	 // Calculate the backoff delay
					next_discovery_check_in_time = t_now_ms + backoff_delay;			 // Schedule the next attempt
					attempt_count++;													 // Increment the attempt count
					printf("Retrying with backoff delay of %lums.\n", backoff_delay);
				}
				else {
					printf("Couldn't reach server in time, will try again in a few minutes.\n");
					next_discovery_check_in_time = t_now_ms + DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS;	 // Reset to regular interval after max attempts
					attempt_count = 0;																	 // Reset attempt count
				}
			}

			http_client.end();	// Free resources
		}
		else {
			printf("WiFi not connected before discovery server POST. Retrying in 5 seconds.\n");
			next_discovery_check_in_time = t_now_ms + 5000;	 // Retry in 5 seconds if WiFi is not connected
		}
	}
}

bool load_section_at( char* section_buffer, char* command, int16_t* byte_index ){
	memset(section_buffer, 0, 128);

	int16_t output_index = 0;
	bool solved = false;
	bool hit_end_of_chunk = false;

	while(solved == false){
		char current_byte = command[*byte_index];

		if(current_byte == '~' || current_byte == '\0'){ // 0 is EOF
			hit_end_of_chunk = true;
			solved = true;
		}
		else if(current_byte == '|'){
			hit_end_of_chunk = false;
			solved = true;
		}
		else{
			// read data
			section_buffer[output_index] = current_byte;
			output_index++;
		}

		(*byte_index)++;
	}

	return hit_end_of_chunk;
}

void parse_config_data(char* config_name, char* config_type, char* config_value, char* config_ui_type, char* config_preset_enabled){
	printf("CONFIG DATA: %s|%s|%s|%s|%s\n", config_name, config_type, config_value, config_ui_type, config_preset_enabled);

	if(fastcmp(config_type, "float")){
		float value = atof(config_value);
		printf("FLOAT VALUE: %f\n", value);
		//place_float_in_config_by_key( TBD );
	}
	else if(fastcmp(config_type, "int")){
		int value = atoi(config_value);
		printf("INT VALUE: %i\n", value);
	}
	else if(fastcmp(config_type, "bool")){
		bool value = fastcmp(config_value, "true");
		printf("BOOL VALUE: %i\n", value);
	}
	else if(fastcmp(config_type, "string")){
		printf("STRING VALUE: %s\n", config_value);
	}
	else{
		printf("UNKNOWN CONFIG TYPE: %s\n", config_type);
	}
}

void broadcast_emotiscope_state(){
	char output_string[2048];
	memset(output_string, 0, 2048);

	// Packet Header
	strcat(output_string, "EMO~");
}

void parse_emotiscope_state(char* command, PsychicWebSocketRequest *request){
	printf("Parsing Emotiscope State...\n");
	int16_t num_bytes = strlen(command);
	int16_t byte_index = 0;

	char section_buffer[128];
	memset(section_buffer, 0, 128);

	char chunk_type[32];
	memset(chunk_type, 0, 32);

	static int16_t config_data_index = 0;

	while(byte_index <= num_bytes){
		bool chunk_ended = load_section_at(section_buffer, command, &byte_index);
		printf("LOADED SECTION: %s\n", section_buffer);
		if(fastcmp(chunk_type, "config")){
			static char config_name[32];
			static char config_type[32];
			static char config_value[32];
			static char config_ui_type[32];
			static char config_preset_enabled[32];

			if(config_data_index == 0){
				memcpy(config_name, section_buffer, 32);
				config_data_index = 1;
			}
			else if(config_data_index == 1){
				memcpy(config_type, section_buffer, 32);
				config_data_index = 2;
			}
			else if(config_data_index == 2){
				memcpy(config_value, section_buffer, 32);
				config_data_index = 3;
			}
			else if(config_data_index == 3){
				memcpy(config_ui_type, section_buffer, 32);
				config_data_index = 4;
			}
			else if(config_data_index == 4){
				memcpy(config_preset_enabled, section_buffer, 32);
				config_data_index = 0;

				parse_config_data(config_name, config_type, config_value, config_ui_type, config_preset_enabled);
			}
		}
		else{
			printf("UNKNOWN CHUNK TYPE: %s\n", chunk_type);
		}

		if(chunk_ended == true){
			printf("Chunk ended.\n");

			if(byte_index < num_bytes){
				// Prepare for new chunk type
				memset(chunk_type, 0, 32);
				load_section_at(section_buffer, command, &byte_index);
				memcpy(chunk_type, section_buffer, 32);
				printf("NEW CHUNK TYPE: %s\n");

				config_data_index = 0;
			}
			else{
				printf("EOF\n");
			}
		}
	}
}

void init_web_server() {
	const char *local_hostname = "emotiscope";
	if (MDNS.begin(local_hostname) == true) {
		MDNS.addService("http", "tcp", 80);
	}
	else{
		Serial.println("Error starting mDNS");
	}

	server.config.max_uri_handlers = 40;  // maximum number of .on() calls

	server.listen(80);

	//WEB_VERSION = "?v=" + String(SOFTWARE_VERSION_MAJOR) + "." + String(SOFTWARE_VERSION_MINOR) + "." + String(SOFTWARE_VERSION_PATCH);

	websocket_handler.onOpen([](PsychicWebSocketClient *client) {
		printf("[socket] connection #%i connected from %s\n", client->socket(), client->remoteIP().toString().c_str());
	});

	websocket_handler.onFrame([](PsychicWebSocketRequest *request, httpd_ws_frame *frame) {
		// printf("[socket] #%d sent: %s\n", request->client()->socket(), (char*)frame->payload);
		httpd_ws_type_t frame_type = frame->type;

		// If it's text, it might be a command
		if (frame_type == HTTPD_WS_TYPE_TEXT) {
			char* command = (char*)frame->payload;
			printf("RX: %s\n", command);

			if(command[0] == 'E' && command[1] == 'M' && command[2] == 'O'){
				parse_emotiscope_state(command, request);
			}
			else{
				parse_command(command, request);
			}
		}
		else {
			printf("UNSUPPORTED WS FRAME TYPE: %d\n", (uint8_t)frame->type);
		}

		return ESP_OK;
	});

	websocket_handler.onClose([](PsychicWebSocketClient *client) {
		printf("[socket] connection #%i closed from %s\n", client->socket(), client->remoteIP().toString().c_str());
	});

	server.on("/ws", &websocket_handler);

	server.on("/mac", HTTP_GET, [](PsychicRequest *request) {
   		return request->reply(mac_str);
	});

	server.on("/save-wifi", HTTP_GET, [](PsychicRequest *request) {
		esp_err_t result = ESP_OK;
		String ssid = "";
		String pass = "";

		if(request->hasParam("ssid") == true){
			ssid += request->getParam("ssid")->value();
		}
		else{
			printf("MISSING SSID PARAM!\n");
			return request->reply(400);
		}
		
		if(request->hasParam("pass") == true){
			pass += request->getParam("pass")->value();
		}
		else{
			printf("MISSING PASS PARAM!\n");
			return request->reply(400);
		}

		printf("GOT NEW WIFI CONFIG: '%s|%s'\n", ssid.c_str(), pass.c_str());
		update_network_credentials(ssid, pass);

		return result;
	});

	server.on("/*", HTTP_GET, [](PsychicRequest *request) {
		esp_err_t result = ESP_OK;
		String path = "";

		char url[128] = { 0 };
		request->url().toCharArray(url, 128);

		// Remove queries
		fetch_substring(url, '?', 0);

		if(fastcmp(substring, "/")){
			path += "/index.html";
			//path += WEB_VERSION;
		}
		else if(fastcmp(substring, "/remote")){
			path += "/remote.html";
			//path += WEB_VERSION;
		}
		else if(fastcmp(substring, "/wifi-setup")){
			path += "/index.html";
			//path += WEB_VERSION;
		}
		else{
			path += substring;
			//path += WEB_VERSION;
		}

		printf("HTTP GET %s\n", path.c_str());

		File file = LittleFS.open(path);
		if (file) {
			String etagStr(file.getLastWrite(), 10);

			PsychicFileResponse response(request, file, path);
			response.addHeader("Cache-Control", "public, max-age=900");
			response.addHeader("ETag", etagStr.c_str());
			result = response.send();
			file.close();
		}
		else {
			result = request->reply(404);
			printf("404: %s\n", path.c_str());
		}
		return result;
	});

	web_server_ready = true;
}

void get_mac(){
	// Define a variable to hold the MAC address
	uint8_t mac_address[6]; // MAC address is 6 bytes

	// Retrieve the MAC address of the device
	WiFi.macAddress(mac_address);

	// Format the MAC address into the char array
	snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
		mac_address[0], mac_address[1], mac_address[2],
		mac_address[3], mac_address[4], mac_address[5]);

	// Print the MAC address string
	printf("MAC Address: %s\n", mac_str);
}

void init_wifi() {
	esp_wifi_set_ps(WIFI_PS_NONE);

	if(wifi_config_mode == true){
		WiFi.begin("testnet", "testpass");
		get_mac();

		WiFi.softAP("Emotiscope Setup");

		printf("Entered AP Mode: %s\n", WiFi.softAPIP().toString().c_str());

		if (web_server_ready == false) {
			dns_server.start(53, "*", WiFi.softAPIP());
			init_web_server();
		}
	}
	else {
		network_connection_attempts = 0;
		WiFi.begin(wifi_ssid, wifi_pass); 
		printf("Started connection attempt to %s...\n", wifi_ssid);

		get_mac();
	}
}

void handle_wifi() {
	static int16_t connection_status_last = -1;
	static uint32_t last_reconnect_attempt = 0;
	const uint32_t reconnect_interval_ms = 5000;

	connection_status = WiFi.status();

	// WiFi status has changed
	if (connection_status != connection_status_last) {
		// Emotiscope connected sucessfully to your network
		if (connection_status == WL_CONNECTED) {
			printf("CONNECTED TO %s SUCCESSFULLY @ %s\n", wifi_ssid, WiFi.localIP().toString().c_str());
		}

		// Emotiscope disconnected from a network
		else if (connection_status == WL_DISCONNECTED) {
			printf("DISCONNECTED FROM WIFI!\n");
		}

		// Emotiscope wireless functions are IDLE
		else if (connection_status == WL_IDLE_STATUS) {
			printf("WIFI IN IDLE STATE.\n");
		}

		// Emotiscope failed to connect to your network
		else if (connection_status == WL_CONNECT_FAILED) {
			printf("FAILED TO CONNECT TO %s\n", wifi_ssid);
		}

		// Emotiscope lost connection to your network
		else if (connection_status == WL_CONNECTION_LOST) {
			printf("LOST CONNECTION TO %s\n", wifi_ssid);
		}

		// Emotiscope can't see your network
		else if (connection_status == WL_NO_SSID_AVAIL) {
			printf("UNABLE TO REACH SSID %s\n", wifi_ssid);
		}

		// Anything else
		else {
			printf("WIFI STATUS CHANGED TO UNHANDLED STATE: %i\n", connection_status);
		}

		if (connection_status == WL_CONNECTED && connection_status_last != WL_CONNECTED) {
			printf("NOW CONNECTED TO NETWORK\n");
			// print_filesystem();
			if (web_server_ready == false) {
				init_web_server();
			}
		}

		else if (connection_status != WL_CONNECTED && connection_status_last == WL_CONNECTED) {
			printf("LOST CONNECTION TO NETWORK, RETRYING\n");
			WiFi.disconnect();
		}
	}
	else if (connection_status != WL_CONNECTED && millis() - last_reconnect_attempt >= reconnect_interval_ms) {
		if(wifi_config_mode == false){
			printf("ATTEMPTING TO RECONNECT TO THE NETWORK\n");
			last_reconnect_attempt = millis();
			WiFi.reconnect();

			network_connection_attempts++;

			if(network_connection_attempts >= 3){
				reboot_into_wifi_config_mode();
			}
		}
		else{
			printf("WIFI CONFIG MODE ACTIVE, NOT RECONNECTING\n");
		}
	}

	connection_status_last = connection_status;
}
