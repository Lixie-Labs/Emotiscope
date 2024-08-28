//#define USB_DEBUG_MODE (true) // Uncomment during development, comment out for production so that Improv WiFi works as intended

#ifdef USB_DEBUG_MODE
	#warning "LIXIE LABS --- USB_DEBUG_MODE is enabled, Improv WiFi will not work as intended! Disable this for production."
#endif

#define USB_BUF_SIZE (1024)
#define USB_HISTORY_LENGTH 256

typedef struct {
	char* ssid;
	int32_t rssi;
	bool auth_required;
} wifi_network_t;

typedef enum {
	IMPROV_CURRENT_STATE_NULL_1,
	IMPROV_CURRENT_STATE_NULL_2,
	IMPROV_CURRENT_STATE_READY,
	IMPROV_CURRENT_STATE_PROVISIONING,
	IMPROV_CURRENT_STATE_PROVISIONED,
} improv_current_state_t;

typedef enum {
	IMPROV_ERROR_NONE,
	IMPROV_ERROR_INVALID_RPC_PACKET,
	IMPROV_ERROR_UNKNOWN_RPC_COMMAND,
	IMPROV_ERROR_UNABLE_TO_CONNNECT
} improv_error_state_t;

typedef enum {
	IMPROV_PACKET_TYPE_NULL,
	IMPROV_PACKET_TYPE_CURRENT_STATE,
	IMPROV_PACKET_TYPE_ERROR_STATE,
	IMPROV_PACKET_TYPE_RPC_COMMAND,
	IMPROV_PACKET_TYPE_RPC_RESULT
} improv_packet_type_t;

typedef enum {
	IMPROV_RPC_COMMAND_NULL,
	IMPROV_RPC_COMMAND_SEND_WIFI_SETTINGS,
	IMPROV_RPC_COMMAND_REQUEST_CURRENT_STATE,
	IMPROV_RPC_COMMAND_REQUEST_DEVICE_INFORMATION,
	IMPROV_RPC_COMMAND_REQUEST_SCANNED_NETWORKS
} improv_rpc_command_t;

// USB
uint8_t usb_rx_buffer[USB_BUF_SIZE];
uint8_t usb_history[USB_HISTORY_LENGTH];
uint16_t usb_history_items = 0;

// UART
const int uart_buffer_size = (1024 * 2);
QueueHandle_t uart_queue;

// Improv
improv_current_state_t improv_current_state = IMPROV_CURRENT_STATE_READY;
improv_error_state_t   improv_error_state = IMPROV_ERROR_NONE;

void uart_print(const char* str) {
	#ifndef USB_DEBUG_MODE
		uart_write_bytes(UART_NUM_0, str, strlen(str));
	#endif
}

void uart_printc(char c) {
	#ifndef USB_DEBUG_MODE
		uart_write_bytes(UART_NUM_0, &c, 1);
	#endif
}

void uart_printd(uint32_t num) {
	#ifndef USB_DEBUG_MODE
		char str[16];
		sprintf(str, "%lu", num);
		uart_write_bytes(UART_NUM_0, str, strlen(str));
	#endif
}

void usb_print(const char* str) {
	#ifndef USB_DEBUG_MODE
		usb_serial_jtag_write_bytes(str, strlen(str), 20);
	#endif
}

void usb_printc(char c) {
	#ifndef USB_DEBUG_MODE
		usb_serial_jtag_write_bytes(&c, 1, 20);
	#endif
}

void reset_usb_history(){
	uart_print("RESET USB HISTORY\n");
	memset(usb_history, 0, 256);
}

void shift_history_left(uint8_t* array, uint16_t array_size, uint16_t shift_amount) {
	// Use memmove to shift array contents to the left
	memmove(array, array + shift_amount, (array_size - shift_amount) * sizeof(uint8_t));
}

void transmit_packet(uint8_t* packet, uint16_t packet_length) {
	#ifndef USB_DEBUG_MODE
		usb_serial_jtag_write_bytes(packet, packet_length, 20);

		uart_print("TRANSMIT PACKET: ");
		for(uint16_t i = 0; i < packet_length; i++){
			uart_printd(packet[i]);
			uart_printc(' ');
		}
		uart_printc('\n');
	#endif
}

void calc_checksum(uint8_t* packet, uint16_t packet_length) {
	uint8_t checksum = 0;
	for(uint16_t i = 0; i < packet_length-2; i++){
		checksum += packet[i];
	}

	packet[packet_length-2] = checksum;
	packet[packet_length-1] = 10;
}

void send_error_state(improv_error_state_t error_state) {
	uint8_t error_state_packet[12] = {
		'I', 'M', 'P', 'R', 'O', 'V',
		1, // Version
		IMPROV_PACKET_TYPE_ERROR_STATE, // Type
		1, // Length
		error_state,
		0,  // Checksum
		10, // Padding (Thank you Julie!)
	};

	calc_checksum(error_state_packet, sizeof(error_state_packet));
	transmit_packet(error_state_packet, sizeof(error_state_packet));
}

void set_improv_error_state(improv_error_state_t error_state) {
	if (error_state == improv_error_state){
		return;
	}
	else{
		improv_error_state = error_state;
		send_error_state(error_state);
	}
}

void send_device_information(uint8_t command_responding_to){
	char*   firmware_name        = "Emotiscope FW";
	uint8_t firmware_name_length = strlen(firmware_name);

	char*  firmware_version         = "2.0.0";
	uint8_t firmware_version_length = strlen(firmware_version);

	char*   hardware_name        = "ESPS3-S3";
	uint8_t hardware_name_length = strlen(hardware_name);

	char* device_name          = "Emotiscope";
	uint8_t device_name_length = strlen(device_name);

	uint8_t total_strings_length = firmware_name_length + firmware_version_length + hardware_name_length + device_name_length;

	uint8_t device_information_packet[total_strings_length + 17];
	device_information_packet[0]  = 'I';
	device_information_packet[1]  = 'M';
	device_information_packet[2]  = 'P';
	device_information_packet[3]  = 'R';
	device_information_packet[4]  = 'O';
	device_information_packet[5]  = 'V';
	device_information_packet[6]  = 1;  // Version
	device_information_packet[7]  = IMPROV_PACKET_TYPE_RPC_RESULT; // Type
	device_information_packet[8]  = total_strings_length + 6; // Length
	device_information_packet[9]  = command_responding_to;
	device_information_packet[10] = total_strings_length+4; // Data length

	uint8_t current_index = 11;

	device_information_packet[current_index] = firmware_name_length;
	memcpy(&device_information_packet[current_index+1], firmware_name, firmware_name_length);
	current_index += firmware_name_length + 1;

	device_information_packet[current_index] = firmware_version_length;
	memcpy(&device_information_packet[current_index+1], firmware_version, firmware_version_length);
	current_index += firmware_version_length + 1;

	device_information_packet[current_index] = hardware_name_length;
	memcpy(&device_information_packet[current_index+1], hardware_name, hardware_name_length);
	current_index += hardware_name_length + 1;

	device_information_packet[current_index] = device_name_length;
	memcpy(&device_information_packet[current_index+1], device_name, device_name_length);
	current_index += device_name_length + 1;

	calc_checksum(device_information_packet, sizeof(device_information_packet));

	transmit_packet(device_information_packet, sizeof(device_information_packet));

	set_improv_error_state(IMPROV_ERROR_NONE);
}

void send_scanned_networks(uint8_t command_responding_to){
	wifi_network_t scanned_networks[3] = {
		{"WilsonAve",  -50, true},
		{"testnet2",   -60, true},
		{"Dinkleberg", -70, true}
	};
	uint8_t num_networks = sizeof(scanned_networks) / sizeof(wifi_network_t);

	for(uint8_t i = 0; i < num_networks; i++){
		char* ssid = scanned_networks[i].ssid;
		uint8_t ssid_length = strlen(ssid);

		char rssi[10];
		snprintf(rssi, 10, "%li", scanned_networks[i].rssi);
		uint8_t rssi_length = strlen(rssi);

		char* auth_required = scanned_networks[i].auth_required ? "YES" : "NO";
		uint8_t auth_required_length = strlen(auth_required);

		uart_print("SSID: ");
		uart_print(ssid);
		uart_print(" len: ");
		uart_printd(ssid_length);
		uart_printc('\n');

		uart_print("RSSI: ");
		uart_print(rssi);
		uart_print(" len: ");
		uart_printd(rssi_length);
		uart_printc('\n');

		uart_print("AUTH REQUIRED: ");
		uart_print(auth_required);
		uart_print(" len: ");
		uart_printd(auth_required_length);
		uart_printc('\n');

		uint8_t rpc_data_length = 1 + ssid_length + 1 + rssi_length + 1 + auth_required_length;

		uint8_t network_information_packet[11 + rpc_data_length + 2];

		network_information_packet[0]  = 'I';
		network_information_packet[1]  = 'M';
		network_information_packet[2]  = 'P';
		network_information_packet[3]  = 'R';
		network_information_packet[4]  = 'O';
		network_information_packet[5]  = 'V';
		network_information_packet[6]  = 1;  // Version
		network_information_packet[7]  = IMPROV_PACKET_TYPE_RPC_RESULT; // Type
		network_information_packet[8]  = rpc_data_length + 2; // Length
		network_information_packet[9]  = command_responding_to;
		network_information_packet[10] = rpc_data_length; // Data length

		uint8_t current_index = 11;

		network_information_packet[current_index] = ssid_length;
		memcpy(&network_information_packet[current_index+1], ssid, ssid_length);
		current_index += ssid_length + 1;

		network_information_packet[current_index] = rssi_length;
		memcpy(&network_information_packet[current_index+1], rssi, rssi_length);
		current_index += rssi_length + 1;

		network_information_packet[current_index] = auth_required_length;
		memcpy(&network_information_packet[current_index+1], auth_required, auth_required_length);
		current_index += auth_required_length + 1;

		calc_checksum(network_information_packet, sizeof(network_information_packet));

		transmit_packet(network_information_packet, sizeof(network_information_packet));
	}

	// Send empty packet to signal end of network list
	uint8_t network_information_packet[13];
	network_information_packet[0]  = 'I';
	network_information_packet[1]  = 'M';
	network_information_packet[2]  = 'P';
	network_information_packet[3]  = 'R';
	network_information_packet[4]  = 'O';
	network_information_packet[5]  = 'V';
	network_information_packet[6]  = 1;  // Version
	network_information_packet[7]  = IMPROV_PACKET_TYPE_RPC_RESULT; // Type
	network_information_packet[8]  = 2; // Length
	network_information_packet[9]  = command_responding_to;
	network_information_packet[10] = 0; // Data length
	calc_checksum(network_information_packet, sizeof(network_information_packet));
	transmit_packet(network_information_packet, sizeof(network_information_packet));
}

void send_redirect_url(uint8_t command_responding_to){
	uint8_t network_information_packet[39];

	network_information_packet[0]  = 'I';
	network_information_packet[1]  = 'M';
	network_information_packet[2]  = 'P';
	network_information_packet[3]  = 'R';
	network_information_packet[4]  = 'O';
	network_information_packet[5]  = 'V';
	network_information_packet[6]  = 1;  // Version
	network_information_packet[7]  = IMPROV_PACKET_TYPE_RPC_RESULT; // Type
	network_information_packet[8]  = 28; // Length
	network_information_packet[9]  = command_responding_to;
	network_information_packet[10] = 26; // Data length
	network_information_packet[11] = 25; // Length of URL
	network_information_packet[12] = 'h';
	network_information_packet[13] = 't';
	network_information_packet[14] = 't';
	network_information_packet[15] = 'p';
	network_information_packet[16] = ':';
	network_information_packet[17] = '/';
	network_information_packet[18] = '/';
	network_information_packet[19] = 'r';
	network_information_packet[20] = '.';
	network_information_packet[21] = 'e';
	network_information_packet[22] = 'm';
	network_information_packet[23] = 'o';
	network_information_packet[24] = 't';
	network_information_packet[25] = 'i';
	network_information_packet[26] = 's';
	network_information_packet[27] = 'c';
	network_information_packet[28] = 'o';
	network_information_packet[29] = 'p';
	network_information_packet[30] = 'e';
	network_information_packet[31] = '.';
	network_information_packet[32] = 'r';
	network_information_packet[33] = 'o';
	network_information_packet[34] = 'c';
	network_information_packet[35] = 'k';
	network_information_packet[36] = 's';

	calc_checksum(network_information_packet, sizeof(network_information_packet));
	transmit_packet(network_information_packet, sizeof(network_information_packet));
}

void send_current_state() {
	uint8_t current_state_packet[12] = {
		'I', 'M', 'P', 'R', 'O', 'V',
		1, // Version
		IMPROV_PACKET_TYPE_CURRENT_STATE, // Type
		1, // Length
		improv_current_state,
		0,  // Checksum
		10, // Padding
	};

	calc_checksum(current_state_packet, sizeof(current_state_packet));
	transmit_packet(current_state_packet, sizeof(current_state_packet));
}

void parse_wifi_settings(uint8_t* packet, uint8_t packet_data_length){
	uint8_t ssid_length = packet[11];
	char ssid[ssid_length+1];
	memcpy(ssid, &packet[12], ssid_length);
	ssid[ssid_length] = '\0';

	uint8_t password_length = packet[12+ssid_length];
	char password[password_length+1];
	memcpy(password, &packet[13+ssid_length], password_length);
	password[password_length] = '\0';

	uart_print("SSID: ");
	uart_print(ssid);
	uart_printc('\n');

	uart_print("PASSWORD: ");
	uart_print(password);
	uart_printc('\n');

	extern void save_wifi_credentials(char* ssid, char* password);
	save_wifi_credentials(ssid, password);

	improv_current_state = IMPROV_CURRENT_STATE_PROVISIONING;
	send_current_state();

	esp_wifi_stop();
	extern void connect_wifi();
	connect_wifi();	
}

void parse_improv_packet(uint8_t* packet, uint8_t packet_length, improv_packet_type_t packet_type, uint8_t packet_data_length) {
	uart_print("PARSE IMPROV PACKET: ");

	for(uint16_t i = 0; i < packet_length; i++){
		uart_printd(packet[i]);
		uart_printc(' ');
	}
	uart_printc('\n');

	if(packet_type == IMPROV_PACKET_TYPE_CURRENT_STATE){
		uart_print("CURRENT STATE\n");
	}
	else if(packet_type == IMPROV_PACKET_TYPE_ERROR_STATE){
		uart_print("ERROR STATE\n");
	}
	else if(packet_type == IMPROV_PACKET_TYPE_RPC_COMMAND){
		improv_rpc_command_t rpc_command = packet[9];
		uint8_t rpc_data_length = packet[10];

		uart_print("RPC COMMAND: ");
		uart_printd(rpc_command);
		uart_printc('\n');

		uart_print("RPC DATA LENGTH: ");
		uart_printd(rpc_data_length);
		uart_printc('\n');

		if(rpc_command == IMPROV_RPC_COMMAND_SEND_WIFI_SETTINGS){
			uart_print("SEND WIFI SETTINGS\n");
			parse_wifi_settings(packet, packet_data_length);
		}
		else if(rpc_command == IMPROV_RPC_COMMAND_REQUEST_CURRENT_STATE){
			uart_print("REQUEST CURRENT STATE\n");
			send_current_state();
			if(improv_current_state == IMPROV_CURRENT_STATE_PROVISIONED){
				send_redirect_url(rpc_command);
			}
		}
		else if(rpc_command == IMPROV_RPC_COMMAND_REQUEST_DEVICE_INFORMATION){
			uart_print("REQUEST DEVICE INFORMATION\n");
			send_device_information(IMPROV_RPC_COMMAND_REQUEST_DEVICE_INFORMATION);
		}
		else if(rpc_command == IMPROV_RPC_COMMAND_REQUEST_SCANNED_NETWORKS){
			uart_print("REQUEST SCANNED NETWORKS\n");
			send_scanned_networks(IMPROV_RPC_COMMAND_REQUEST_SCANNED_NETWORKS);
		}
		else{
			set_improv_error_state(IMPROV_ERROR_UNKNOWN_RPC_COMMAND);
		}
	}
	else if(packet_type == IMPROV_PACKET_TYPE_RPC_RESULT){
		uart_print("RPC RESULT\n");
	}
}

void search_for_improv_packet(){
	if (usb_history_items < 6) {
		return;
	}

	for(uint16_t i = 0; i < (USB_HISTORY_LENGTH)-8; i++){
		if(
			usb_history[i+0] == 'I' &&
			usb_history[i+1] == 'M' &&
			usb_history[i+2] == 'P' &&
			usb_history[i+3] == 'R' &&
			usb_history[i+4] == 'O' &&
			usb_history[i+5] == 'V'
		){
			uart_print("\nSAW IMPROV HEADER: ");
			uint16_t packet_index = i;
			uint8_t bytes_left = (USB_HISTORY_LENGTH) - packet_index;

			for(uint16_t j = 0; j < 6; j++){
				uart_printd(usb_history[packet_index+j]);
				uart_printc(' ');
			}
			uart_printc('\n');

			uint8_t packet_version = usb_history[packet_index+6];
			uint8_t packet_type    = usb_history[packet_index+7];
			uint8_t packet_length  = usb_history[packet_index+8];

			uart_print("PACKET VERSION: ");
			uart_printd(packet_version);
			uart_printc('\n');

			uart_print("PACKET TYPE: ");
			uart_printd(packet_type);
			uart_printc('\n');

			uart_print("PACKET LENGTH: ");
			uart_printd(packet_length);
			uart_printc('\n');

			uint8_t bytes_needed = 9 + packet_length + 2;

			if (bytes_left == bytes_needed){
				// Parse packet
				uint8_t* packet_pointer = &usb_history[packet_index];
				parse_improv_packet(packet_pointer, bytes_needed, packet_type, packet_length);

				reset_usb_history();
			}
			else{
				uart_print("----- PACKET INCOMPLETE!!!\n");
			}

			break;
		}
	}
}

void store_usb_data(char c) {
	//usb_print("NEW CHAR:");
	//usb_print(&c);

	shift_history_left(usb_history, 256, 1);
	usb_history[255] = c;

	usb_history_items++;
	if (usb_history_items > 256) {
		usb_history_items = 256;
	}
}

void init_serial(){
	#ifdef USB_DEBUG_MODE
		esp_log_level_set("*", ESP_LOG_INFO);
	#else
		const uart_port_t uart_num = UART_NUM_0;
		uart_config_t uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
		};
		// Configure UART parameters
		ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
		ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

		// Configure USB SERIAL JTAG
		usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
			.rx_buffer_size = USB_BUF_SIZE,
			.tx_buffer_size = USB_BUF_SIZE,
		};

		ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
		//fsync(fileno(stdout)); // flush stdout

		reset_usb_history();
	#endif
}

void check_serial(){
	#ifndef USB_DEBUG_MODE
		static int64_t last_state_send = 0;
		int64_t t_now_serial = esp_timer_get_time();
		if(t_now_serial - last_state_send > 3000000){
			//send_current_state();
			last_state_send = t_now_serial;
		}

		// Read data from USB
		memset(usb_rx_buffer, 0, USB_BUF_SIZE);
		volatile int len = usb_serial_jtag_read_bytes(usb_rx_buffer, USB_BUF_SIZE-1, 1);

		if (len) {
			for(uint16_t i = 0; i < len; i++){
				// Convert rx bytes into decimal and print
				uint8_t decimal = usb_rx_buffer[i];
				char str[5];
				sprintf(str, "%d ", decimal);
				//uart_write_bytes(UART_NUM_0, str, strlen(str));
			}

			for(uint16_t i = 0; i < len; i++){
				store_usb_data(usb_rx_buffer[i]);
			}

			search_for_improv_packet();

		}
	#endif
}