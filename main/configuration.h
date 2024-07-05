#define MIN_SAVE_WAIT_MS (3 * 1000)	 // Values must stabilize for this many seconds to be written to NVS

nvs_handle_t config_handle;
config configuration; // configuration struct to be filled by NVS or defaults on boot

volatile int64_t last_save_request_ms = 0;
volatile bool save_request_open = false;
volatile bool filesystem_ready = true;

extern int64_t t_now_ms;

// Put aritrary bytes/blob into NVS
size_t put_bytes(const char *key, const void *value, size_t len) {
	if ( !key || !value || !len ) {
		return 0;
	}

	esp_err_t err = nvs_set_blob(config_handle, key, value, len);
	if (err) {
		ESP_LOGE(TAG, "nvs_set_blob fail: %s %d", key, err);
		return 0;
	}

	err = nvs_commit(config_handle);
	if (err) {
		ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
		return 0;
	}

	return len;
}

// Get the length of a blob in NVS
size_t get_bytes_length(const char *key) {
	size_t len = 0;
	if (!key) {
		return 0;
	}

	esp_err_t err = nvs_get_blob(config_handle, key, NULL, &len);
	if (err) {
		ESP_LOGE(TAG, "nvs_get_blob len fail: %s %d", key, err);
		return 0;
	}

	return len;
}

// Get arbitrary bytes/blob from NVS
size_t get_bytes(const char *key, void *buf, size_t max_len) {
	size_t len = get_bytes_length(key);
	if (!len || !buf || !max_len) {
		return len;
	}

	if (len > max_len) {
		ESP_LOGE(TAG, "not enough space in buffer: %u < %u", max_len, len);
		return 0;
	}
	
	esp_err_t err = nvs_get_blob(config_handle, key, buf, &len);
	if (err) {
		ESP_LOGE(TAG, "nvs_get_blob fail: %s %d", key, err);
		return 0;
	}
	
	return len;
}

// Put a float into NVS
size_t put_float(const char *key, const float value) {
	return put_bytes(key, (void *)&value, sizeof(float));
}

// Get a float from NVS
float get_float(const char *key, const float default_value) {
	float value = default_value;
	
	if( get_bytes(key, (void *)&value, sizeof(float)) == 0 ){
		put_float(key, default_value);
	}
	
	return value;
}

// Put a long into NVS
size_t put_long(const char *key, int32_t value) {
	if ( !key ) {
		return 0;
	}
	
	esp_err_t err = nvs_set_i32(config_handle, key, value);
	if (err) {
		ESP_LOGE(TAG, "nvs_set_i32 fail: %s %d", key, err);
		return 0;
	}
	
	err = nvs_commit(config_handle);
	if (err) {
		ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
		return 0;
	}
	
	return 4;
}

// Get a long from NVS
int32_t get_long(const char *key, const int32_t default_value) {
	int32_t value = default_value;
	if ( !key) {
		return value;
	}

	esp_err_t err = nvs_get_i32(config_handle, key, &value);
	if (err) {
		ESP_LOGE(TAG, "nvs_get_i32 fail: %s %d", key, err);
		put_long(key, default_value);
	}

	return value;
}

// Put a ulong into NVS
size_t put_ulong(const char *key, uint32_t value) {
	if ( !key ) {
		return 0;
	}

	esp_err_t err = nvs_set_u32(config_handle, key, value);
	if (err) {
		ESP_LOGE(TAG, "nvs_set_u32 fail: %s %d", key, err);
		return 0;
	}

	err = nvs_commit(config_handle);
	if (err) {
		ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
		return 0;
	}
	
	return 4;
}

// Get a ulong from NVS
uint32_t get_ulong(const char *key, const uint32_t default_value) {
	uint32_t value = default_value;
	if ( !key ) {
		return value;
	}

	esp_err_t err = nvs_get_u32(config_handle, key, &value);
	if (err) {
		ESP_LOGE(TAG, "nvs_get_u32 fail: %s %d", key, err);
		put_ulong(key, default_value);
	}

	return value;
}

// Get a string from NVS
size_t get_string(const char *key, char *value, const size_t max_len) {
	size_t len = 0;
	if (!key || !value || !max_len) {
		return 0;
	}
	
	esp_err_t err = nvs_get_str(config_handle, key, NULL, &len);
	if (err) {
		ESP_LOGE(TAG, "nvs_get_str len fail: %s %d", key, err);
		return 0;
	}
	
	if (len > max_len) {
		ESP_LOGE(TAG, "not enough space in value: %u < %u", max_len, len);
		return 0;
	}
	
	err = nvs_get_str(config_handle, key, value, &len);
	if (err) {
		ESP_LOGE(TAG, "nvs_get_str fail: %s %d", key, err);
		return 0;
	}
	
	return len;
}

// Put a string into NVS
size_t put_string(const char *key, const char *value) {
	if ( !key || !value ) {
		return 0;
	}

	esp_err_t err = nvs_set_str(config_handle, key, value);
	if (err) {
		ESP_LOGE(TAG, "nvs_set_str fail: %s %d", key, err);
		return 0;
	}
	
	err = nvs_commit(config_handle);
	if (err) {
		ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
		return 0;
	}
	
	return strlen(value);
}

// Initialize a config item with a name and defaults
config_item init_config_item(char* name, char* pretty_name, char* type_string, char* ui_type_string, type_t type, ui_type_t ui_type, config_value default_value){
	config_item item;
	strcpy(item.name, name);
	strcpy(item.pretty_name, pretty_name);
	strcpy(item.type_string, type_string);
	strcpy(item.ui_type_string, ui_type_string);
	item.type = type;
	item.ui_type = ui_type;

	dsps_memcpy_aes3(&item.value, &default_value, sizeof(config_value));

	if(type == u32t){
		item.value.u32 = get_ulong(name, default_value.u32);
		ESP_LOGI(TAG, "Loaded %s from NVS as u32: %lu", name, item.value.u32);
		
	} else if(type == i32t){
		item.value.i32 = get_long(name, default_value.i32);
		ESP_LOGI(TAG, "Loaded %s from NVS as i32: %li", name, item.value.i32);
	} else if(type == f32t){
		item.value.f32 = get_float(name, default_value.f32);
		ESP_LOGI(TAG, "Loaded %s from NVS as f32: %.3f", name, item.value.f32);
	}
	
	return item;
}

void load_configuration_defaults(){
	// This is where defaults are stored! Config items must be manually initialized here
	// before being loaded from NVS. There are three datatypes: u32, i32, and f32.
	// These correspond to uint32_t, int32_t, and float, respectively. Bools are stored
	// as u32s.
	//
	// To add a new config item, add it to the config struct in types.h, then add a
	// matching loader to this function. The loader should look like the ones below,
	// defining the name, pretty_name, type, ui_type, and default value of the config item.
	// 
	// The configuration can be accessed in two ways: at compile time by key name
	// (configuration.brightness) or at runtime by iteration with pointer offsets
	// (config_item my_config_item = *(config_location + i))

	configuration.brightness = init_config_item(
		"brightness",
		"Brightness",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 1.00} // <-- Default value
	);
	
	configuration.softness = init_config_item(
		"softness",
		"Softness",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.25} // <-- Default value
	);

	configuration.color = init_config_item(
		"color",
		"Color",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.33} // <-- Default value
	);

	configuration.color_range = init_config_item(
		"color_range",
		"Color Range",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.00} // <-- Default value
	);

	configuration.warmth = init_config_item(
		"warmth",
		"Warmth",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.50} // <-- Default value
	);

	configuration.speed = init_config_item(
		"speed",
		"Speed",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.50} // <-- Default value
	);

	configuration.saturation = init_config_item(
		"saturation",
		"Saturation",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.95} // <-- Default value
	);

	configuration.background = init_config_item(
		"background",
		"Background",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.25} // <-- Default value
	);

	configuration.current_mode = init_config_item(
		"current_mode",
		"Current Mode",
		"u32",
		"t",
		u32t,
		ui_type_menu_toggle,
		(config_value){.u32 = 1} // <-- Default value
	);

	configuration.mirror_mode = init_config_item(
		"mirror_mode",
		"Mirror Mode",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = 1} // <-- Default value
	);

	configuration.screensaver = init_config_item(
		"screensaver",
		"Screensaver",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = 1} // <-- Default value
	);

	configuration.temporal_dithering = init_config_item(
		"dithering",
		"Temporal Dithering",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = 1} // <-- Default value
	);

	configuration.reverse_color_range = init_config_item(
		"reverse_color",
		"Reverse Color Range",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = 0} // <-- Default value
	);

	configuration.auto_color_cycle = init_config_item(
		"auto_color",
		"Auto Color Cycle",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = 0} // <-- Default value
	);

	configuration.blur = init_config_item(
		"blur",
		"Blur",
		"f32",
		"s",
		f32t,
		ui_type_slider,
		(config_value){.f32 = 0.00} // <-- Default value
	);

	configuration.show_ui = init_config_item(
		"show_ui",
		"Show UI",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = 1} // <-- Default value
	);

	configuration.color_mode = init_config_item(
		"color_mode",
		"Color Mode",
		"u32",
		"t",
		u32t,
		ui_type_toggle,
		(config_value){.u32 = COLOR_MODE_GRADIENT} // <-- Default value
	);
}

// Initialize NVS and the configuration struct with all config items
void init_configuration(){
	ESP_LOGI(TAG, "init_configuration()");

	// Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

	ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    err = nvs_open("emotiscope", NVS_READWRITE, &config_handle);
	if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }

	// Load config from NVS
	dsps_memset_aes3(&configuration, 0, sizeof(configuration)); // Clear the configuration struct

	load_configuration_defaults(); // Load default values into the configuration struct
}

// Save configuration to LittleFS
bool save_config() {
	config_item* config_location = (config_item*)(&configuration);
	uint16_t num_config_items = sizeof(configuration) / sizeof(config_item);

	for(uint16_t i = 0; i < num_config_items; i++){
		config_item item = *(config_location + i);
		type_t type = item.type;

		if(type == u32t){
			put_ulong(item.name, item.value.u32);
		} else if(type == i32t){
			put_long(item.name, item.value.i32);
		} else if(type == f32t){
			put_float(item.name, item.value.f32);
		} else {
			ESP_LOGE(TAG, "Unknown type in save_config");
			return false;
		}
	}

	return true;
}

void save_config_delayed() {
	last_save_request_ms = t_now_ms;
	save_request_open = true;
}

void sync_configuration_to_file_system() {
	start_profile(__COUNTER__, __func__);
	if (save_request_open == true) {
		if ((t_now_ms - last_save_request_ms) >= MIN_SAVE_WAIT_MS) {
			filesystem_ready = false;
			ESP_LOGI(TAG, "SAVING NVS");
			save_config();
			save_request_open = false;
			filesystem_ready = true;
		}
	}
	end_profile();
}