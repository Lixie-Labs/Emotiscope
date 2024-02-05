#define CONFIG_FILENAME "/configuration.bin"
#define MIN_SAVE_WAIT_MS ( 3 * 1000 ) // Values must stabilize for this many seconds to be written to FS

extern lightshow_mode lightshow_modes[];
extern PsychicWebSocketHandler websocket_handler;

config configuration = {
	1.00, // .... brightness
	8.00, // .... speed
	0.00, // .... hue	
	0, // ....... current_mode
	true, // .... mirror_mode
	0.6, // ..... incandescent_filter
	0.0, // ..... hue_range
};

uint32_t last_save_request_ms = 0;
bool save_request_open = false;

void sync_configuration_to_client(){
	char config_item_buffer[120];

	websocket_handler.sendAll("clear_config");

	// brightness
	snprintf(config_item_buffer, 120, "new_config|brightness|float|%.3f", configuration.brightness);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);

	// speed
	snprintf(config_item_buffer, 120, "new_config|speed|float|%.3f", configuration.speed);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);	

	// hue
	snprintf(config_item_buffer, 120, "new_config|hue|float|%.3f", configuration.hue);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);	

	// current_mode
	snprintf(config_item_buffer, 120, "new_config|current_mode|int|%li", configuration.current_mode);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);	

	// mirror_mode
	snprintf(config_item_buffer, 120, "new_config|mirror_mode|int|%d", configuration.mirror_mode);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);	

	// incandescent_filter
	snprintf(config_item_buffer, 120, "new_config|incandescent|float|%.3f", configuration.incandescent_filter);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);	

	// hue_range
	snprintf(config_item_buffer, 120, "new_config|hue_range|float|%.3f", configuration.hue_range);
	websocket_handler.sendAll(config_item_buffer); memset(config_item_buffer, 0, 120);	

	websocket_handler.sendAll("config_ready");
}

// Save configuration to LittleFS
bool save_config() {
  File file = LittleFS.open(CONFIG_FILENAME, FILE_WRITE);
  if (!file) {
    printf("Failed to open %s for writing!", CONFIG_FILENAME);
    return false;
  } else {
    const uint8_t* ptr = (const uint8_t*)&configuration;

	// Iterate over the size of config and write each byte to the file
	for (size_t i = 0; i < sizeof(config); i++) {
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
  } else {
    // Ensure the configuration structure is sized properly
    if (file.size() != sizeof(config)) {
      printf("Config file size does not match config structure size!\n");
      file.close();
      return false;
    }

    uint8_t* ptr = (uint8_t*)&configuration; // Pointer to the configuration structure

    // Read the file content into the configuration structure
    for (size_t i = 0; i < sizeof(config); i++) {
      int byte = file.read(); // Read a byte
      if (byte == -1) { // Check for read error or end of file
        printf("Error reading %s!\n", CONFIG_FILENAME);
        break;
      }
      ptr[i] = (uint8_t)byte; // Store the byte in the configuration structure
    }
  }
  file.close(); // Close the file after reading

  return true;
}

void save_config_delayed(){
	last_save_request_ms = millis();
	save_request_open = true;
}

void sync_configuration_to_file_system(uint32_t t_now_ms){
	if(save_request_open == true){
		if((t_now_ms - last_save_request_ms) >= MIN_SAVE_WAIT_MS){
			printf("SAVING CONFIGURATION TO FILESYSTEM\n");
			save_config();
			save_request_open = false;
		}
	}
}

void init_configuration() {
	// Attempt to load config from flash
	printf("LOADING CONFIG...");
	bool load_success = load_config();

	// If we couldn't load the file, save a fresh copy
	if(load_success == false){
		printf("FAIL\n");
		save_config();
	}
	else{
		printf("PASS\n");
	}
}