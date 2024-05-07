#define NOISE_SPECTRUM_FILENAME "/noise_spectrum.bin"
#define NETWORK_CONFIG_FILENAME "/secrets/network.txt"
#define AUDIO_DEBUG_RECORDING_FILENAME "/audio.bin"
#define MIN_SAVE_WAIT_MS (3 * 1000)	 // Values must stabilize for this many seconds to be written to NVS

#define MAX_AUDIO_RECORDING_SAMPLES ( 12800 * 3 ) // 3 seconds at [SAMPLE_RATE]

Preferences preferences; // NVS storage for configuration

extern lightshow_mode lightshow_modes[];
extern PsychicWebSocketHandler websocket_handler;

volatile bool wifi_config_mode = false;

config configuration; // configuration struct to be filled by NVS or defaults on boot

float noise_spectrum[NUM_FREQS] = {0.0};

int16_t audio_debug_recording[MAX_AUDIO_RECORDING_SAMPLES];
uint32_t audio_recording_index = 0;
volatile bool audio_recording_live = false;

volatile uint32_t last_save_request_ms = 0;
volatile bool save_request_open = false;

volatile bool filesystem_ready = true;

void load_config(){
	// Load configuration from NVS

	// Brightness
	configuration.brightness = preferences.getFloat("brightness", 1.00);

	// Softness
	configuration.softness = preferences.getFloat("softness", 0.25);

	// Color
	configuration.color = preferences.getFloat("color", 0.90);

	// Color Range
	configuration.color_range = preferences.getFloat("color_range", 0.00);

	// Blue Filter
	configuration.blue_filter = preferences.getFloat("blue_filter", 0.00);

	// Speed
	configuration.speed = preferences.getFloat("speed", 0.75);

	// Saturation
	configuration.saturation = preferences.getFloat("saturation", 1.00);

	// Background
	configuration.background = preferences.getFloat("background", 0.00);

	// Current Mode
	configuration.current_mode = preferences.getInt("current_mode", 0);

	// Mirror Mode
	configuration.mirror_mode = preferences.getBool("mirror_mode", true);

	// Screensaver
	configuration.screensaver = preferences.getBool("screensaver", true);

	// Temporal Dithering
	configuration.temporal_dithering = preferences.getBool("dithering", true);

	// VU Floor
	configuration.vu_floor = preferences.getFloat("vu_floor", 0.00);

	// Ambient Left Threshold
	configuration.ambient_left_threshold = preferences.getULong("al_threshold", 33000);

	// Ambient Center Threshold
	configuration.ambient_center_threshold = preferences.getULong("ac_threshold", 95000);

	// Ambient Right Threshold
	configuration.ambient_right_threshold = preferences.getULong("ar_threshold", 64000);

	// Touch Left Threshold
	configuration.touch_left_threshold = preferences.getULong("tl_threshold", 33000*2);

	// Touch Center Threshold
	configuration.touch_center_threshold = preferences.getULong("tc_threshold", 95000*2);

	// Touch Right Threshold
	configuration.touch_right_threshold = preferences.getULong("tr_threshold", 64000*2);

	// Reverse Color
	configuration.reverse_color_range = preferences.getBool("reverse_color", false);

	// Auto Color Cycling
	configuration.auto_color_cycle = preferences.getBool("auto_color", false);
}

void sync_configuration_to_client() {
	char config_item_buffer[120];

	websocket_handler.sendAll("clear_config");

	// brightness
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|brightness|float|%.3f", configuration.brightness);
	websocket_handler.sendAll(config_item_buffer);

	// softness
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|softness|float|%.3f", configuration.softness);
	websocket_handler.sendAll(config_item_buffer);

	// speed
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|speed|float|%.3f", configuration.speed);
	websocket_handler.sendAll(config_item_buffer);

	// color
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|color|float|%.3f", configuration.color);
	websocket_handler.sendAll(config_item_buffer);

	// current_mode
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|current_mode|int|%li", configuration.current_mode);
	websocket_handler.sendAll(config_item_buffer);

	// mirror_mode
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|mirror_mode|int|%d", configuration.mirror_mode);
	websocket_handler.sendAll(config_item_buffer);

	// blue_filter
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|blue_filter|float|%.3f", configuration.blue_filter);
	websocket_handler.sendAll(config_item_buffer);

	// color_range
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|color_range|float|%.3f", configuration.color_range);
	websocket_handler.sendAll(config_item_buffer);

	// saturation
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|saturation|float|%.3f", configuration.saturation);
	websocket_handler.sendAll(config_item_buffer);

	// background
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|background|float|%.3f", configuration.background);
	websocket_handler.sendAll(config_item_buffer);

	// screensaver
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|screensaver|int|%d", configuration.screensaver);
	websocket_handler.sendAll(config_item_buffer);

	// temporal_dithering
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|temporal_dithering|int|%d", configuration.temporal_dithering);
	websocket_handler.sendAll(config_item_buffer);

	// reverse_color_range
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|reverse_color_range|int|%d", configuration.reverse_color_range);
	websocket_handler.sendAll(config_item_buffer);

	// auto_color_cycle
	memset(config_item_buffer, 0, 120);
	snprintf(config_item_buffer, 120, "new_config|auto_color_cycle|int|%d", configuration.auto_color_cycle);
	websocket_handler.sendAll(config_item_buffer);

	websocket_handler.sendAll("config_ready");
}

// Save configuration to LittleFS
bool save_config() {
	preferences.putFloat("brightness", configuration.brightness);
	preferences.putFloat("softness",   configuration.softness);
	preferences.putFloat("color", configuration.color);
	preferences.putFloat("color_range", configuration.color_range);
	preferences.putFloat("blue_filter", configuration.blue_filter);
	preferences.putFloat("speed", configuration.speed);
	preferences.putFloat("saturation", configuration.saturation);
	preferences.putFloat("background", configuration.background);
	preferences.putInt("current_mode", configuration.current_mode);
	preferences.putBool("mirror_mode", configuration.mirror_mode);
	preferences.putBool("screensaver", configuration.screensaver);
	preferences.putBool("dithering", configuration.temporal_dithering);
	preferences.putFloat("vu_floor", configuration.vu_floor);
	preferences.putULong("al_threshold", configuration.ambient_left_threshold);
	preferences.putULong("ac_threshold", configuration.ambient_center_threshold);
	preferences.putULong("ar_threshold", configuration.ambient_right_threshold);
	preferences.putULong("tl_threshold", configuration.touch_left_threshold);
	preferences.putULong("tc_threshold", configuration.touch_center_threshold);
	preferences.putULong("tr_threshold", configuration.touch_right_threshold);
	preferences.putBool("reverse_color", configuration.reverse_color_range);
	preferences.putBool("auto_color", configuration.auto_color_cycle);

	return true;
}

// Save noise spectrum to LittleFS
bool save_noise_spectrum() {
	File file = LittleFS.open(NOISE_SPECTRUM_FILENAME, FILE_WRITE);
	if (!file) {
		printf("Failed to open %s for writing!", NOISE_SPECTRUM_FILENAME);
		return false;
	}
	else {
		const uint8_t* ptr = (const uint8_t*)&noise_spectrum;

		// Iterate over the size of noise_spectrum and write each byte to the file
		for (size_t i = 0; i < sizeof(float) * NUM_FREQS; i++) {
			file.write(ptr[i]);
		}
	}
	file.close();
	return true;
}

// Load noise_spectrum from LittleFS
bool load_noise_spectrum() {
	// Open the file for reading
	File file = LittleFS.open(NOISE_SPECTRUM_FILENAME, FILE_READ);
	if (!file) {
		printf("Failed to open %s for reading!\n", NOISE_SPECTRUM_FILENAME);
		return false;
	}
	else {
		// Ensure the noise_spectrum array is sized properly
		if (file.size() != sizeof(float) * NUM_FREQS) {
			printf("Noise spectrum size does not match expected size! (%zu != %zu)\n", file.size(), sizeof(float) * NUM_FREQS);
			file.close();
			return false;
		}

		uint8_t* ptr = (uint8_t*)&noise_spectrum;  // Pointer to the noise_spectrum array

		// Read the file content into the noise_spectrum array
		for (size_t i = 0; i < sizeof(float) * NUM_FREQS; i++) {
			int byte = file.read();	 // Read a byte
			if (byte == -1) {		 // Check for read error or end of file
				printf("Error reading %s!\n", NOISE_SPECTRUM_FILENAME);
				break;
			}
			ptr[i] = (uint8_t)byte;	 // Store the byte in the noise_spectrum array
		}
	}
	file.close();  // Close the file after reading
	return true;
}

void save_config_delayed() {
	last_save_request_ms = t_now_ms;
	save_request_open = true;
}

void sync_configuration_to_file_system() {
	if (save_request_open == true) {
		if ((t_now_ms - last_save_request_ms) >= MIN_SAVE_WAIT_MS) {
			filesystem_ready = false;
			printf("SAVING\n");
			save_config();
			//save_noise_spectrum();
			save_request_open = false;
			filesystem_ready = true;
		}
	}
}

// Function to update the network credentials and restart the ESP
void update_network_credentials(String new_ssid, String new_pass) {
	// store the new credentials in the global variables
	memset(wifi_ssid, 0, 64);
	memset(wifi_pass, 0, 64);
	new_ssid.toCharArray(wifi_ssid, 64);
	new_pass.toCharArray(wifi_pass, 64);

	// Update the network credentials in NVS
	preferences.putBytes("wifi_ssid", wifi_ssid, 64);
	preferences.putBytes("wifi_pass", wifi_pass, 64);

    // Print a success message
    printf("Network credentials updated successfully! Restarting to attempt connection\n");

    // Delay for 100 ms
    delay(100);

    // Restart the ESP
    ESP.restart();
}


void load_network_credentials(){
	memset(wifi_ssid, 0, 64);
	memset(wifi_pass, 0, 64);

	// Load network credentials from NVS with Preferences library, just like the config struct using getBytes on the char arrays:
	// SSID
	preferences.getBytes("wifi_ssid", wifi_ssid, 64);
	// Password
	preferences.getBytes("wifi_pass", wifi_pass, 64);
}

void init_configuration() {
	preferences.begin("emotiscope", false);

	// Check if wifi config mode was requested
	bool wifi_config_mode_trigger = preferences.getBool("CONFIG_TRIG", false);
	if (wifi_config_mode_trigger) {
		preferences.putBool("CONFIG_TRIG", false);
		wifi_config_mode = true;
	}

	// Attempt to load config from flash
	printf("LOADING CONFIG...");
	load_config();
	printf("PASS\n");

	// Attempt to load noise_spectrum from flash
	printf("LOADING NOISE SPECTRUM...");
	bool load_success = load_noise_spectrum();

	// If we couldn't load the file, save a fresh copy
	if (load_success == false) {
		printf("FAIL\n");
		save_noise_spectrum();
	}
	else {
		printf("PASS\n");
	}

	// Attempt to load wifi creds from flash
	printf("LOADING NETWORK...");
	load_network_credentials();
	printf("PASS\n");
}

// Save debug audio recording to LittleFS
bool save_audio_debug_recording() {
	File file = LittleFS.open(AUDIO_DEBUG_RECORDING_FILENAME, FILE_WRITE);
	if (!file) {
		printf("Failed to open %s for writing!", AUDIO_DEBUG_RECORDING_FILENAME);
		return false;
	}
	else {
		const uint8_t* ptr = (const uint8_t*)&audio_debug_recording;

		// Iterate over the size of noise_spectrum and write each byte to the file
		for (size_t i = 0; i < sizeof(int16_t) * MAX_AUDIO_RECORDING_SAMPLES; i++) {
			file.write(ptr[i]);
		}

		printf("Audio debug recording saved!\n");
	}
	file.close();
	return true;
}