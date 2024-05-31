#define DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS (10 * (1000 * 60))	 // "10" is minutes
#define MAX_HTTP_REQUEST_ATTEMPTS (8)								 // Define the maximum number of retry attempts
#define INITIAL_BACKOFF_MS (1000)									 // Initial backoff delay in milliseconds
#define MAX_NETWORK_CONNECT_ATTEMPTS (3)
#define MAX_WEBSOCKETS_CLIENTS (4)

#define DISCOVERY_SERVER_URL "https://app.emotiscope.rocks/discovery/"

String WEB_VERSION = "";

DNSServer dns_server; // DNS server instance

// Define a char array to hold the formatted MAC address string
char mac_str[18]; // MAC address string format "XX:XX:XX:XX:XX:XX" + '\0'

PsychicHttpServer server;
PsychicWebSocketHandler websocket_handler;
PsychicEventSource event_source;

volatile bool web_server_ready = false;
int16_t connection_status = -1;

uint8_t network_connection_attempts = 0;

websocket_client clients[MAX_WEBSOCKETS_CLIENTS];

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

	//printf("LOADED SECTION: %s\n", section_buffer);

	return hit_end_of_chunk;
}

void set_config_value_by_pretty_name(char* config_pretty_name, char* new_config_value){
	config_item* config_location = reinterpret_cast<config_item*>(&configuration);
	const uint16_t num_config_items = sizeof(configuration) / sizeof(config_item);

	for(uint16_t i = 0; i < num_config_items; i++){
		config_item* item = config_location + i;
		type_t type = item->type;

		if(fastcmp(item->pretty_name, config_pretty_name)){
			if(type == u32){
				item->value.u32 = atoi(new_config_value);
				//printf("Setting u32 %s: %d\n", item->name, item->value.u32);
			} else if(type == i32){
				item->value.i32 = atoi(new_config_value);
				//printf("Setting i32 %s: %d\n", item->name, item->value.i32);
			} else if(type == f32){
				item->value.f32 = atof(new_config_value);
				//printf("Setting f32 %s: %f\n", item->name, item->value.f32);
			} else {
				printf("ERROR: Unknown type in set_config_value_by_pretty_name\n");
			}

			break;
		}
	}
}

void broadcast_emotiscope_state(){
	char output_string[2048];
	char temp_buffer[128];

	// Stats
	memset(output_string, 0, 2048);
	memset(temp_buffer, 0, 128);

	strcat(output_string, "EMO");
	strcat(output_string, "~stats");

	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|cpu_usage|%.3f", CPU_CORE_USAGE);
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|cpu_temp|%.3f", CPU_TEMP);
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|fps_cpu|%.3f", FPS_CPU);
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|fps_gpu|%.3f", FPS_GPU);
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|heap|%lu", esp_get_free_heap_size());
	strcat(output_string, temp_buffer);

	uint16_t current_ws_clients = 0;
	for(uint16_t i = 0; i < MAX_WEBSOCKETS_CLIENTS; i++){
		if(clients[i].last_proof_of_life != 0){
			current_ws_clients++;
		}
	}
	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|ws_clients|%lu", current_ws_clients);
	strcat(output_string, temp_buffer);

	//printf("TX: %s\n", output_string);
	event_source.send(output_string);

	// Configuration
	memset(output_string, 0, 2048);
	memset(temp_buffer, 0, 128);

	strcat(output_string, "EMO");
	strcat(output_string, "~config");

	config_item* config_location = reinterpret_cast<config_item*>(&configuration);
	uint16_t num_config_items = sizeof(configuration) / sizeof(config_item);

	for(uint16_t i = 0; i < num_config_items; i++){
		config_item item = *(config_location + i);
		
		strcat(output_string, "|");
		strcat(output_string, item.pretty_name);
		strcat(output_string, "|");
		strcat(output_string, item.type_string);
		strcat(output_string, "|");
		strcat(output_string, item.ui_type_string);
		strcat(output_string, "|");

		if(item.type == u32){
			memset(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%lu", item.value.u32);
			strcat(output_string, temp_buffer);
		}
		else if(item.type == i32){
			memset(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%li", item.value.i32);
			strcat(output_string, temp_buffer);
		}
		else if(item.type == f32){
			memset(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%.3f", item.value.f32);
			strcat(output_string, temp_buffer);
		}
		else{
			printf("ERROR: Unknown type in broadcast_emotiscope_state\n");
		}
	}

	//printf("TX: %s\n", output_string);
	event_source.send(output_string);

	// Light Modes
	memset(output_string, 0, 2048);
	memset(temp_buffer, 0, 128);

	strcat(output_string, "EMO");
	strcat(output_string, "~modes");

	for(uint16_t i = 0; i < NUM_LIGHT_MODES; i++){
		light_mode_type_t type = light_modes[i].type;
		if(type != LIGHT_MODE_TYPE_SYSTEM){
			strcat(output_string, "|");
			strcat(output_string, light_modes[i].name);
			strcat(output_string, "|");

			if(type == LIGHT_MODE_TYPE_ACTIVE){
				strcat(output_string, "0");
			}
			else if(type == LIGHT_MODE_TYPE_INACTIVE){
				strcat(output_string, "1");
			}
			else{
				printf("ERROR: Unknown light mode type in broadcast_emotiscope_state\n");
			}
		}
	}

	//printf("TX: %s\n", output_string);
	event_source.send(output_string);
}

void parse_emotiscope_packet(char* command, PsychicWebSocketRequest *request){
	//printf("Parsing Emotiscope Packet...\n");
	int16_t num_bytes = strlen(command);
	int16_t byte_index = 0;

	char section_buffer[128];
	memset(section_buffer, 0, 128);

	char chunk_type[32];
	memset(chunk_type, 0, 32);

	int16_t config_data_index = 0;

	while(byte_index <= num_bytes){
		bool chunk_ended = load_section_at(section_buffer, command, &byte_index);

		if(fastcmp(chunk_type, "set_config")){
			static char config_pretty_name[32];
			static char new_config_value[32];

			if(config_data_index == 0){
				memcpy(config_pretty_name, section_buffer, 32);
				config_data_index = 1;
			}
			else if(config_data_index == 1){
				memcpy(new_config_value, section_buffer, 32);
				config_data_index = 0;

				set_config_value_by_pretty_name(config_pretty_name, new_config_value);
				save_config_delayed();
			}
		}
		else{
			if(chunk_type[0] == '\0'){
				//printf("PACKET HEADER: %s\n", section_buffer);	
			}
			else{
				printf("UNKNOWN CHUNK TYPE: %s\n", chunk_type);
			}
		}

		if(chunk_ended == true){
			//printf("Chunk ended.\n");

			if(byte_index < num_bytes){
				// Prepare for new chunk type
				memset(chunk_type, 0, 32);
				load_section_at(section_buffer, command, &byte_index);
				memcpy(chunk_type, section_buffer, 32);
				printf("PACKET CHUNK TYPE: %s\n", chunk_type);

				config_data_index = 0;
			}
			else{
				//printf("EOF\n");
			}
		}
	}
}

bool add_client(PsychicWebSocketClient *client){
	bool found_slot = false;
	for(uint16_t i = 0; i < MAX_WEBSOCKETS_CLIENTS; i++){
		if(clients[i].last_proof_of_life == 0){
			clients[i].socket = client->socket();
			clients[i].last_proof_of_life = t_now_ms;
			found_slot = true;
			break;
		}
	}

	return found_slot;
}

void remove_client(int socket){
	for(uint16_t i = 0; i < MAX_WEBSOCKETS_CLIENTS; i++){
		if(clients[i].socket == socket){
			clients[i].last_proof_of_life = 0;
			clients[i].socket = 0;
			break;
		}
	}
}

void test_clients(){
	for(uint16_t i = 0; i < MAX_WEBSOCKETS_CLIENTS; i++){
		if(clients[i].last_proof_of_life != 0){
			if(t_now_ms - clients[i].last_proof_of_life >= 200){
				printf("Client #%i has not sent proof of life in 200ms, closing.\n", clients[i].socket);
				PsychicWebSocketClient *client = websocket_handler.getClient(clients[i].socket);
				if(client != NULL){
					client->close();
				}
				else{
					printf("ERROR: Client not found in test_clients\n");
				}

				clients[i].last_proof_of_life = 0;
				clients[i].socket = 0;
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
		printf("Error starting mDNS\n");
	}

	memset(clients, 0, sizeof(clients));

	server.config.stack_size = 8192;	 // stack size for each thread
	server.config.max_uri_handlers = 20;  // maximum number of .on() calls
	server.config.max_open_sockets = 8;   // maximum number of open sockets

	server.listen(80);

	//WEB_VERSION = "?v=" + String(SOFTWARE_VERSION_MAJOR) + "." + String(SOFTWARE_VERSION_MINOR) + "." + String(SOFTWARE_VERSION_PATCH);

	event_source.onOpen([](PsychicEventSourceClient *client) {
		printf("[eventsource] connection #%u connected from %s\n", client->socket(), client->remoteIP().toString());
	});

	event_source.onClose([](PsychicEventSourceClient *client) {
		printf("[eventsource] connection #%u closed from %s\n", client->socket(), client->remoteIP().toString());
	});

	//attach the handler to /events
	server.on("/events", &event_source);

	websocket_handler.onOpen([](PsychicWebSocketClient *client) {
		printf("[socket] connection #%i connected from %s\n", client->socket(), client->remoteIP().toString().c_str());

		if( add_client( client ) == false ){
			printf( "Client not accepted, too many clients.\n" );
			client->close();
		}
	});

	websocket_handler.onFrame([](PsychicWebSocketRequest *request, httpd_ws_frame *frame) {
		// printf("[socket] #%d sent: %s\n", request->client()->socket(), (char*)frame->payload);
		httpd_ws_type_t frame_type = frame->type;

		for(uint16_t i = 0; i < MAX_WEBSOCKETS_CLIENTS; i++){
			if(clients[i].socket == request->client()->socket()){
				clients[i].last_proof_of_life = t_now_ms;
			}
		}

		// If it's text, it might be a command
		if (frame_type == HTTPD_WS_TYPE_TEXT) {
			char* command = (char*)frame->payload;
			printf("RX: %s\n", command);

			if(command[0] == 'E' && command[1] == 'M' && command[2] == 'O'){
				parse_emotiscope_packet(command, request);
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
		remove_client(client->socket());
	});

	server.on("/ws", &websocket_handler);
	//server.on("/ws")->attachHandler(&websocket_handler);

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
