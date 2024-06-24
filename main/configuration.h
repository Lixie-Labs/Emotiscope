nvs_handle_t config_handle;
config configuration; // configuration struct to be filled by NVS or defaults on boot

void load_configuration(){
	ESP_LOGI(TAG, "load_configuration()");

	esp_err_t err;
	// This is where defaults are stored! Config items must be manually initialized here
	// before being loaded from NVS. There are three datatypes: u32, i32, and f32.
	// These correspond to uint32_t, int32_t, and float, respectively. Bools are stored
	// as u32s.
	//
	// To add a new config item, add it to the config struct in types.h, then add a
	// matching loader to this function. The loader should look like the ones below,
	// defining the name, pretty_name, type, ui_type, and default value of the config item.
	// 
	// The configuration can be accessed in two ways: by key name (configuration.brightness)
	// or by iteration with pointer offsets (config_item my_config_item = *(config_location + i))

	memset(&configuration, 0, sizeof(configuration)); // Clear the configuration struct

	// Brightness
	strcpy(configuration.brightness.name, "brightness");
	strcpy(configuration.brightness.pretty_name, "Brightness");	
	strcpy(configuration.brightness.type_string, "f32");
	strcpy(configuration.brightness.ui_type_string, "s");
	configuration.brightness.type = f32;
	configuration.brightness.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.brightness.name, &configuration.brightness.value.f32);
	if (err != ESP_OK) {
		configuration.brightness.value.f32 = 1.00;
	}

	// Softness
	strcpy(configuration.softness.name, "softness");
	strcpy(configuration.softness.pretty_name, "Softness");
	strcpy(configuration.softness.type_string, "f32");
	strcpy(configuration.softness.ui_type_string, "s");
	configuration.softness.type = f32;
	configuration.softness.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.softness.name, &configuration.softness.value.f32);
	if (err != ESP_OK) {
		configuration.softness.value.f32 = 0.25;
	}

	// Color
	strcpy(configuration.color.name, "color");
	strcpy(configuration.color.pretty_name, "Color");
	strcpy(configuration.color.type_string, "f32");
	strcpy(configuration.color.ui_type_string, "s");
	configuration.color.type = f32;
	configuration.color.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.color.name, &configuration.color.value.f32);
	if (err != ESP_OK) {
		configuration.color.value.f32 = 0.33;
	}

	// Color Range
	strcpy(configuration.color_range.name, "color_range");
	strcpy(configuration.color_range.pretty_name, "Color Range");
	strcpy(configuration.color_range.type_string, "f32");
	strcpy(configuration.color_range.ui_type_string, "s");
	configuration.color_range.type = f32;
	configuration.color_range.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.color_range.name, &configuration.color_range.value.f32);
	if (err != ESP_OK) {
		configuration.color_range.value.f32 = 0.00;
	}

	// Warmth
	strcpy(configuration.warmth.name, "warmth");
	strcpy(configuration.warmth.pretty_name, "Warmth");
	strcpy(configuration.warmth.type_string, "f32");
	strcpy(configuration.warmth.ui_type_string, "s");
	configuration.warmth.type = f32;
	configuration.warmth.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.warmth.name, &configuration.warmth.value.f32);
	if (err != ESP_OK) {
		configuration.warmth.value.f32 = 0.50;
	}

	// Speed
	strcpy(configuration.speed.name, "speed");
	strcpy(configuration.speed.pretty_name, "Speed");
	strcpy(configuration.speed.type_string, "f32");
	strcpy(configuration.speed.ui_type_string, "s");
	configuration.speed.type = f32;
	configuration.speed.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.speed.name, &configuration.speed.value.f32);
	if (err != ESP_OK) {
		configuration.speed.value.f32 = 0.50;
	}

	// Saturation
	strcpy(configuration.saturation.name, "saturation");
	strcpy(configuration.saturation.pretty_name, "Saturation");
	strcpy(configuration.saturation.type_string, "f32");
	strcpy(configuration.saturation.ui_type_string, "s");
	configuration.saturation.type = f32;
	configuration.saturation.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.saturation.name, &configuration.saturation.value.f32);
	if (err != ESP_OK) {
		configuration.saturation.value.f32 = 0.95;
	}

	// Background
	strcpy(configuration.background.name, "background");
	strcpy(configuration.background.pretty_name, "Background");
	strcpy(configuration.background.type_string, "f32");
	strcpy(configuration.background.ui_type_string, "s");
	configuration.background.type = f32;
	configuration.background.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.background.name, &configuration.background.value.f32);
	if (err != ESP_OK) {
		configuration.background.value.f32 = 0.00;
	}

	// Current Mode
	strcpy(configuration.current_mode.name, "current_mode");
	strcpy(configuration.current_mode.pretty_name, "Current Mode");
	strcpy(configuration.current_mode.type_string, "u32");
	strcpy(configuration.current_mode.ui_type_string, "n");
	configuration.current_mode.type = u32;
	configuration.current_mode.ui_type = ui_type_none;
	err = nvs_get_u32(config_handle, configuration.current_mode.name, &configuration.current_mode.value.u32);
	if (err != ESP_OK) {
		configuration.current_mode.value.u32 = 0;
	}
		
	// Mirror Mode
	strcpy(configuration.mirror_mode.name, "mirror_mode");
	strcpy(configuration.mirror_mode.pretty_name, "Mirror Mode");
	strcpy(configuration.mirror_mode.type_string, "u32");
	strcpy(configuration.mirror_mode.ui_type_string, "t");
	configuration.mirror_mode.type = u32;
	configuration.mirror_mode.ui_type = ui_type_toggle;
	err = nvs_get_u32(config_handle, configuration.mirror_mode.name, &configuration.mirror_mode.value.u32);
	if (err != ESP_OK) {
		configuration.mirror_mode.value.u32 = 1;
	}

	// Screensaver
	strcpy(configuration.screensaver.name, "screensaver");
	strcpy(configuration.screensaver.pretty_name, "Screensaver");
	strcpy(configuration.screensaver.type_string, "u32");
	strcpy(configuration.screensaver.ui_type_string, "mt");
	configuration.screensaver.type = u32;
	configuration.screensaver.ui_type = ui_type_menu_toggle;
	err = nvs_get_u32(config_handle, configuration.screensaver.name, &configuration.screensaver.value.u32);
	if (err != ESP_OK) {
		configuration.screensaver.value.u32 = 1;
	}

	// Temporal Dithering
	strcpy(configuration.temporal_dithering.name, "dithering");
	strcpy(configuration.temporal_dithering.pretty_name, "Temporal Dithering");
	strcpy(configuration.temporal_dithering.type_string, "u32");
	strcpy(configuration.temporal_dithering.ui_type_string, "mt");
	configuration.temporal_dithering.type = u32;
	configuration.temporal_dithering.ui_type = ui_type_menu_toggle;
	err = nvs_get_u32(config_handle, configuration.temporal_dithering.name, &configuration.temporal_dithering.value.u32);
	if (err != ESP_OK) {
		configuration.temporal_dithering.value.u32 = 1;
	}
	
	// Reverse Color
	strcpy(configuration.reverse_color_range.name, "reverse_color");
	strcpy(configuration.reverse_color_range.pretty_name, "Reverse Color Range");
	strcpy(configuration.reverse_color_range.type_string, "u32");
	strcpy(configuration.reverse_color_range.ui_type_string, "t");
	configuration.reverse_color_range.type = u32;
	configuration.reverse_color_range.ui_type = ui_type_toggle;
	err = nvs_get_u32(config_handle, configuration.reverse_color_range.name, &configuration.reverse_color_range.value.u32);
	if (err != ESP_OK) {
		configuration.reverse_color_range.value.u32 = 0;
	}

	// Auto Color Cycling
	strcpy(configuration.auto_color_cycle.name, "auto_color");
	strcpy(configuration.auto_color_cycle.pretty_name, "Auto Color Cycle");
	strcpy(configuration.auto_color_cycle.type_string, "u32");
	strcpy(configuration.auto_color_cycle.ui_type_string, "t");
	configuration.auto_color_cycle.type = u32;
	configuration.auto_color_cycle.ui_type = ui_type_toggle;
	err = nvs_get_u32(config_handle, configuration.auto_color_cycle.name, &configuration.auto_color_cycle.value.u32);
	if (err != ESP_OK) {
		configuration.auto_color_cycle.value.u32 = 0;
	}

	// Blur
	strcpy(configuration.blur.name, "blur");
	strcpy(configuration.blur.pretty_name, "Blur");	
	strcpy(configuration.blur.type_string, "f32");
	strcpy(configuration.blur.ui_type_string, "s");
	configuration.blur.type = f32;
	configuration.blur.ui_type = ui_type_slider;
	err = nvs_get_u32(config_handle, configuration.blur.name, &configuration.blur.value.f32);
	if (err != ESP_OK) {
		configuration.blur.value.f32 = 0.00;
	}

	// Show Interface
	strcpy(configuration.show_ui.name, "show_ui");
	strcpy(configuration.show_ui.pretty_name, "Show Interface");
	strcpy(configuration.show_ui.type_string, "u32");
	strcpy(configuration.show_ui.ui_type_string, "mt");
	configuration.show_ui.type = u32;
	configuration.show_ui.ui_type = ui_type_menu_toggle;
	err = nvs_get_u32(config_handle, configuration.show_ui.name, &configuration.show_ui.value.u32);
	if (err != ESP_OK) {
		configuration.show_ui.value.u32 = 1;
	}
}

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
    } else {
        ESP_LOGI(TAG, "Done");
	}

	/*
	// Read
	printf("Reading restart counter from NVS ... ");
	int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
	err = nvs_get_i32(config_handle, "restart_counter", &restart_counter);
	switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("Restart counter = %" PRIu32 "\n", restart_counter);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
	}

	// Write
	printf("Updating restart counter in NVS ... ");
	restart_counter++;
	err = nvs_set_i32(config_handle, "restart_counter", restart_counter);
	printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

	// Commit written value.
	// After setting any values, nvs_commit() must be called to ensure changes are written
	// to flash storage. Implementations may write to storage at other times,
	// but this is not guaranteed.
	printf("Committing updates in NVS ... ");
	err = nvs_commit(config_handle);
	printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
	*/

	// Load config from NVS
	load_configuration();
}