#define DISCOVERY_CHECK_IN_INTERVAL_MILLISECONDS (10 * (1000 * 60))	 // "10" is minutes
#define MAX_HTTP_REQUEST_ATTEMPTS (8)								 // Define the maximum number of retry attempts
#define INITIAL_BACKOFF_MS (1000)									 // Initial backoff delay in milliseconds
#define MAX_NETWORK_CONNECT_ATTEMPTS (3)

#define DISCOVERY_SERVER_URL "https://app.emotiscope.rocks/discovery/"

String WEB_VERSION = "";

const IPAddress ap_ip(192, 168, 4, 1); // IP address for the ESP32-S3 in AP mode
const IPAddress ap_gateway(192, 168, 4, 1); // Gateway IP address, same as ESP32-S3 IP
const IPAddress ap_subnet(255, 255, 255, 0); // Subnet mask for the WiFi network
DNSServer dns_server; // DNS server instance

// Define a char array to hold the formatted MAC address string
char mac_str[18]; // MAC address string format "XX:XX:XX:XX:XX:XX" + '\0'

// HTTPS not working yet, PsychicHTTP can't initialize the SSL server
bool app_enable_ssl = true;
const char server_cert[] = "-----BEGIN CERTIFICATE-----\n"
	"MIIEIjCCAwqgAwIBAgISBH7YjHKyJu9WiS8x5r5AoQBbMA0GCSqGSIb3DQEBCwUA\n"
	"MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD\n"
	"EwJSMzAeFw0yNDAzMTIyMjI1MTBaFw0yNDA2MTAyMjI1MDlaMBsxGTAXBgNVBAMT\n"
	"EGVtb3Rpc2NvcGUucm9ja3MwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATQHjaY\n"
	"I+CdZFEl7b4uLJQvM9wc4PQuP3bwNYT22xgF+vMqZan+dFPQ2aivqTQTmfpZf7P4\n"
	"i5Zabvke7fLcVgL6o4ICEjCCAg4wDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQG\n"
	"CCsGAQUFBwMBBggrBgEFBQcDAjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBTKbZV5\n"
	"T6G16rxt/6U4ceZ47dH38DAfBgNVHSMEGDAWgBQULrMXt1hWy65QCUDmH6+dixTC\n"
	"xjBVBggrBgEFBQcBAQRJMEcwIQYIKwYBBQUHMAGGFWh0dHA6Ly9yMy5vLmxlbmNy\n"
	"Lm9yZzAiBggrBgEFBQcwAoYWaHR0cDovL3IzLmkubGVuY3Iub3JnLzAbBgNVHREE\n"
	"FDASghBlbW90aXNjb3BlLnJvY2tzMBMGA1UdIAQMMAowCAYGZ4EMAQIBMIIBBAYK\n"
	"KwYBBAHWeQIEAgSB9QSB8gDwAHYASLDja9qmRzQP5WoC+p0w6xxSActW3SyB2bu/\n"
	"qznYhHMAAAGONPvyqwAABAMARzBFAiByFzLVHoKxCHjMzswH9uorSMDLaRT7R0Qd\n"
	"p7GS/wRmxAIhAO+vULdM8/l57nfNbHTO7ZDaPNaHdXtnpB2iPZl1VV+RAHYA7s3Q\n"
	"ZNXbGs7FXLedtM0TojKHRny87N7DUUhZRnEftZsAAAGONPvyZQAABAMARzBFAiEA\n"
	"0N6+Jcg5MIQSY8npJf+z6Sos+YvL6oAgBct8ho45J5kCIF4W1k1QmJSDDbT7UvI5\n"
	"6vM3mNME7+FDsv7Dx+SxXJpHMA0GCSqGSIb3DQEBCwUAA4IBAQAWNeX3A+1elo4H\n"
	"HclveVrcw1vbJfJWIfN+GYr6EXzWlUtDWHQNzpwNZWy5KhizypJV2nKEMEaZrkMp\n"
	"hg0nVfU1EIlT7gDmLrxLneZMig5G1HFuikf5iS28qasG+WWwlR6lOPKWmnGb+Eyg\n"
	"N7KpKPOolfggrmt1n1PjR3CEI9b31ISNW1WiedFZf0WKfva8yhjH+vqM8H179z+H\n"
	"j3Ly0aEo80dX4CtPhsvuS//Zp8ICeac6Bp7hiy45hOMJVba7e+khdXQOjA5NIf1w\n"
	"Fg0zi3hBsQ1OuoKirhAXYgMvjhIqVR6hZQCl0Qo04OeGib12o1oIryun9XjElM7A\n"
	"IAEFbV9H\n"
	"-----END CERTIFICATE-----\n";

const char server_key[] = "-----BEGIN PRIVATE KEY-----\n"
	"MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgQhojjmRxKkBrJ2J+\n"
	"N0xPI/w2QqYFwegoEvwHt2pNF/OhRANCAARM1A650C1wbnD3LDFeYEnBYJU9UG8x\n"
	"4fFbE06zxFwt04nZJ9RLHu5uwKffSkZzhOAUAB+EjvA+9x4h4vbAM+nd\n"
	"----------END PRIVATE KEY----------\n";

PsychicHttpServer server;
PsychicWebSocketHandler websocket_handler;
websocket_client websocket_clients[MAX_WEBSOCKET_CLIENTS];

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

int16_t get_slot_of_client(PsychicWebSocketClient client) {
	for (uint16_t i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
		if (websocket_clients[i].socket == client.socket()) {
			return i;
		}
	}

	return -1;
}

PsychicWebSocketClient *get_client_in_slot(uint8_t slot) {
	PsychicWebSocketClient *client = websocket_handler.getClient(websocket_clients[slot].socket);
	if (client != NULL) {
		return client;
	}

	return NULL;
}

void init_websocket_clients() {
	for (uint16_t i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
		websocket_clients[i] = {
			-1,	 // int socket;
			0,	 // uint32_t last_ping;
		};
	}
}

bool welcome_websocket_client(PsychicWebSocketClient client) {
	bool client_welcome_status = true;
	//uint32_t t_now_ms = millis();

	uint16_t current_client_count = 0;
	int16_t first_open_slot = -1;
	for (uint16_t i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
		if (websocket_clients[i].socket != -1) {
			current_client_count += 1;
		}
		else {
			if (first_open_slot == -1) {
				first_open_slot = i;
			}
		}
	}

	// If no room left for new clients
	if (current_client_count >= MAX_WEBSOCKET_CLIENTS || first_open_slot == -1) {
		client_welcome_status = false;
	}

	// If there is room in the party, client is welcome and should be initialized
	if (client_welcome_status == true) {
		websocket_clients[first_open_slot] = {
			client.socket(),  // int socket;
			t_now_ms,		  // uint32_t last_ping;
		};
		printf("PLAYER WELCOMED INTO OPEN SLOT #%i\n", first_open_slot);
	}

	return client_welcome_status;
}

void websocket_client_left(uint16_t client_index) {
	printf("PLAYER #%i LEFT\n", client_index);
	PsychicWebSocketClient *client = get_client_in_slot(client_index);
	if (client != NULL) {
		client->close();
	}

	websocket_clients[client_index].socket = -1;
}

void websocket_client_left(PsychicWebSocketClient client) {
	int socket = client.socket();
	for (uint16_t i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
		if (websocket_clients[i].socket == socket) {
			websocket_client_left((uint16_t)i);
			break;
		}
	}
}

void check_if_websocket_client_still_present(uint16_t client_slot) {
	if (websocket_clients[client_slot].socket != -1) {
		// make sure our client is still connected.
		PsychicWebSocketClient *client = get_client_in_slot(client_slot);
		if (client == NULL) {
			websocket_client_left(client_slot);
		}
	}
}

void transmit_to_client_in_slot(const char *message, uint8_t client_slot) {
	PsychicWebSocketClient *client = get_client_in_slot(client_slot);
	if (client != NULL) {
		client->sendMessage(message);
	}
}

void init_web_server() {
	server.config.max_uri_handlers = 20;  // maximum number of .on() calls

	server.listen(80);

	WEB_VERSION = "?v=" + String(SOFTWARE_VERSION_MAJOR) + "." + String(SOFTWARE_VERSION_MINOR) + "." + String(SOFTWARE_VERSION_PATCH);

	//server.serveStatic("/", LittleFS, "/");

	server.on("/ws", &websocket_handler);

	server.on("/audio", [](PsychicRequest *request) {
		String filename = "/audio.bin";
		PsychicFileResponse response(request, LittleFS, filename);

		return response.send();
	});

	server.on("/mac", HTTP_GET, [](PsychicRequest *request) {
   		return request->reply(mac_str);
	});

	server.on("/save-wifi", HTTP_POST, [](PsychicRequest *request) {
		if(wifi_config_mode == true){
			String ssid = request->getParam("ssid")->value();
			String pass = request->getParam("pass")->value();

			printf("GOT NEW WIFI CONFIG: '%s|%s'\n", ssid.c_str(), pass.c_str());

			update_network_credentials(ssid, pass);

			return request->reply(200);
		}
		else{
			printf("Can't access WIFI config endpoint outside of config AP mode for security reasons!\n");
			return request->reply(400);
		}
	});

	server.on("/*", HTTP_GET, [](PsychicRequest *request) {
		esp_err_t result = ESP_OK;
		String path = "";

		char url[128] = { 0 };
		request->url().toCharArray(url, 128);

		// Remove queries
		fetch_substring(url, '?', 0);

		if(fastcmp(substring, "/")){
			path += "/remote.html";
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

		printf("HTTP GET %s\n", path);

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
		}
		return result;
	});

	const char *local_hostname = "emotiscope";
	if (!MDNS.begin(local_hostname)) {
		Serial.println("Error starting mDNS");
		return;
	}
	MDNS.addService("http", "tcp", 80);

	websocket_handler.onOpen([](PsychicWebSocketClient *client) {
		printf("[socket] connection #%i connected from %s\n", client->socket(), client->remoteIP().toString().c_str());
		if (welcome_websocket_client(client) == true) {
			client->sendMessage("welcome");
		}
		else {
			// Room is full, client not welcome
			printf("PLAYER WAS DENIED ENTRY (ROOM FULL)\n");
			client->close();
		}
	});

	websocket_handler.onFrame([](PsychicWebSocketRequest *request, httpd_ws_frame *frame) {
		// printf("[socket] #%d sent: %s\n", request->client()->socket(), (char*)frame->payload);

		httpd_ws_type_t frame_type = frame->type;

		// If it's text, it might be a command
		if (frame_type == HTTPD_WS_TYPE_TEXT) {
			//printf("RX: %s\n", (char *)frame->payload);
			queue_command((char *)frame->payload, frame->len, get_slot_of_client(request->client()));
		}
		else {
			printf("UNSUPPORTED WS FRAME TYPE: %d\n", (uint8_t)frame->type);
		}

		return ESP_OK;
	});

	websocket_handler.onClose([](PsychicWebSocketClient *client) {
		printf("[socket] connection #%i closed from %s\n", client->socket(), client->remoteIP().toString().c_str());
		websocket_client_left(client);
	});

	init_websocket_clients();

	web_server_ready = true;
}

void init_wifi() {
	if(wifi_config_mode == true){
		WiFi.softAP("Emotiscope Setup");
		dns_server.start(53, "*", WiFi.softAPIP());

		printf("Entered AP Mode: %s\n", WiFi.softAPIP().toString().c_str());

		if (web_server_ready == false) {
			init_web_server();
		}
	}
	else {
		network_connection_attempts = 0;
		WiFi.begin(wifi_ssid, wifi_pass); 
		printf("Started connection attempt to %s...\n", wifi_ssid);
	}

	esp_wifi_set_ps(WIFI_PS_NONE);

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
			//printf("WIFI CONFIG MODE ACTIVE, NOT RECONNECTING\n");
		}
	}

	connection_status_last = connection_status;
}
