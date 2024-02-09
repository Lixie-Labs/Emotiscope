#define CONFIG_FILENAME "/configuration.bin"
#define NOISE_SPECTRUM_FILENAME "/noise_spectrum.bin"
#define AUDIO_DEBUG_RECORDING_FILENAME "/audio.bin"
#define MIN_SAVE_WAIT_MS (3 * 1000)	 // Values must stabilize for this many seconds to be written to FS

#define MAX_AUDIO_RECORDING_SAMPLES ( 12800 * 3 ) // 3 seconds at [SAMPLE_RATE]

extern lightshow_mode lightshow_modes[];
extern PsychicWebSocketHandler websocket_handler;

config configuration = {
	1.00,  // ... brightness
	8.00,  // ... speed
	0.00,  // ... hue
	0,	   // ... current_mode
	true,  // ... mirror_mode
	0.6,   // ... incandescent_filter
	0.0,   // ... hue_range
};

float noise_spectrum[NUM_FREQS] = {0.0};

int16_t audio_debug_recording[MAX_AUDIO_RECORDING_SAMPLES];
uint32_t audio_recording_index = 0;
volatile bool audio_recording_live = false;

uint32_t last_save_request_ms = 0;
bool save_request_open = false;

void sync_configuration_to_client() {
	char config_item_buffer[120];

	websocket_handler.sendAll("clear_config");

	// brightness
	snprintf(config_item_buffer, 120, "new_config|brightness|float|%.3f", configuration.brightness);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	// speed
	snprintf(config_item_buffer, 120, "new_config|speed|float|%.3f", configuration.speed);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	// hue
	snprintf(config_item_buffer, 120, "new_config|hue|float|%.3f", configuration.hue);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	// current_mode
	snprintf(config_item_buffer, 120, "new_config|current_mode|int|%li", configuration.current_mode);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	// mirror_mode
	snprintf(config_item_buffer, 120, "new_config|mirror_mode|int|%d", configuration.mirror_mode);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	// incandescent_filter
	snprintf(config_item_buffer, 120, "new_config|incandescent|float|%.3f", configuration.incandescent_filter);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	// hue_range
	snprintf(config_item_buffer, 120, "new_config|hue_range|float|%.3f", configuration.hue_range);
	websocket_handler.sendAll(config_item_buffer);
	memset(config_item_buffer, 0, 120);

	websocket_handler.sendAll("config_ready");
}

// Save configuration to LittleFS
bool save_config() {
	File file = LittleFS.open(CONFIG_FILENAME, FILE_WRITE);
	if (!file) {
		printf("Failed to open %s for writing!", CONFIG_FILENAME);
		return false;
	}
	else {
		const uint8_t* ptr = (const uint8_t*)&configuration;

		// Iterate over the size of config and write each byte to the file
		for (size_t i = 0; i < sizeof(config); i++) {
			file.write(ptr[i]);
		}
	}
	file.close();
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

// Load configuration from LittleFS
bool load_config() {
	// Open the file for reading
	File file = LittleFS.open(CONFIG_FILENAME, FILE_READ);
	if (!file) {
		printf("Failed to open %s for reading!\n", CONFIG_FILENAME);
		return false;
	}
	else {
		// Ensure the configuration structure is sized properly
		if (file.size() != sizeof(config)) {
			printf("Config file size does not match config structure size!\n");
			file.close();
			return false;
		}

		uint8_t* ptr = (uint8_t*)&configuration;  // Pointer to the configuration structure

		// Read the file content into the configuration structure
		for (size_t i = 0; i < sizeof(config); i++) {
			int byte = file.read();	 // Read a byte
			if (byte == -1) {		 // Check for read error or end of file
				printf("Error reading %s!\n", CONFIG_FILENAME);
				break;
			}
			ptr[i] = (uint8_t)byte;	 // Store the byte in the configuration structure
		}
	}
	file.close();  // Close the file after reading
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
			printf("Noise spectrum size does not match expected size! (%lu != %lu)\n", file.size(), sizeof(float) * NUM_FREQS);
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

// TODO: save_config_delayed() breaks down after a few hours, saves the config on every single frame:
/*
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
[socket] connection #51 connected from 192.168.1.48
WEBSOCKET CLIENT IS WELCOME INTO OPEN SLOT #0
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
SAVING CONFIGURATION TO FILESYSTEM
*/
void save_config_delayed() {
	last_save_request_ms = millis();
	save_request_open = true;
}

// TODO: Display flicker when saving config+noise_spectrum after accepting save request
// Could be RMT doesn't have enough buffer, could be LittleFS takes too long
void sync_configuration_to_file_system(uint32_t t_now_ms) {
	if (save_request_open == true) {
		if ((t_now_ms - last_save_request_ms) >= MIN_SAVE_WAIT_MS) {
			printf("SAVING CONFIGURATION TO FILESYSTEM\n");
			save_config();
			save_noise_spectrum();
			save_request_open = false;
		}
	}
}

void init_configuration() {
	// Attempt to load config from flash
	printf("LOADING CONFIG...");
	bool load_success = load_config();

	// If we couldn't load the file, save a fresh copy
	if (load_success == false) {
		printf("FAIL\n");
		save_config();
	}
	else {
		printf("PASS\n");
	}

	// Attempt to load noise_spectrum from flash
	printf("LOADING NOISE SPECTRUM...");
	load_success = load_noise_spectrum();

	// If we couldn't load the file, save a fresh copy
	if (load_success == false) {
		printf("FAIL\n");
		save_noise_spectrum();
	}
	else {
		printf("PASS\n");
	}
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