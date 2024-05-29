#define NETWORK_CONFIG_FILENAME "/secrets/network.txt"
#define MIN_SAVE_WAIT_MS (3 * 1000)	 // Values must stabilize for this many seconds to be written to NVS

Preferences preferences; // NVS storage for configuration

config configuration; // configuration struct to be filled by NVS or defaults on boot
volatile bool configuration_changed = false; // flag to indicate that the configuration has been changed

extern light_mode light_modes[];
extern PsychicWebSocketHandler websocket_handler;

volatile bool wifi_config_mode = false;

volatile uint32_t last_save_request_ms = 0;
volatile bool save_request_open = false;

volatile bool filesystem_ready = true;

// Load configuration from NVS
void load_config(){
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
	configuration.brightness.value.f32 = preferences.getFloat(configuration.brightness.name, 1.00);

	// Softness
	strcpy(configuration.softness.name, "softness");
	strcpy(configuration.softness.pretty_name, "Softness");
	strcpy(configuration.softness.type_string, "f32");
	strcpy(configuration.softness.ui_type_string, "s");
	configuration.softness.type = f32;
	configuration.softness.ui_type = ui_type_slider;
	configuration.softness.value.f32   = preferences.getFloat(configuration.softness.name, 0.25);

	// Color
	strcpy(configuration.color.name, "color");
	strcpy(configuration.color.pretty_name, "Color");
	strcpy(configuration.color.type_string, "f32");
	strcpy(configuration.color.ui_type_string, "s");
	configuration.color.type = f32;
	configuration.color.ui_type = ui_type_slider;
	configuration.color.value.f32   = preferences.getFloat(configuration.color.name, 0.33);

	// Color Range
	strcpy(configuration.color_range.name, "color_range");
	strcpy(configuration.color_range.pretty_name, "Color Range");
	strcpy(configuration.color_range.type_string, "f32");
	strcpy(configuration.color_range.ui_type_string, "s");
	configuration.color_range.type = f32;
	configuration.color_range.ui_type = ui_type_slider;
	configuration.color_range.value.f32   = preferences.getFloat(configuration.color_range.name, 0.0);

	// Warmth
	strcpy(configuration.warmth.name, "warmth");
	strcpy(configuration.warmth.pretty_name, "Warmth");
	strcpy(configuration.warmth.type_string, "f32");
	strcpy(configuration.warmth.ui_type_string, "s");
	configuration.warmth.type = f32;
	configuration.warmth.ui_type = ui_type_slider;
	configuration.warmth.value.f32   = preferences.getFloat(configuration.warmth.name, 0.50);

	// Speed
	strcpy(configuration.speed.name, "speed");
	strcpy(configuration.speed.pretty_name, "Speed");
	strcpy(configuration.speed.type_string, "f32");
	strcpy(configuration.speed.ui_type_string, "s");
	configuration.speed.type = f32;
	configuration.speed.ui_type = ui_type_slider;
	configuration.speed.value.f32   = preferences.getFloat(configuration.speed.name, 0.50);

	// Saturation
	strcpy(configuration.saturation.name, "saturation");
	strcpy(configuration.saturation.pretty_name, "Saturation");
	strcpy(configuration.saturation.type_string, "f32");
	strcpy(configuration.saturation.ui_type_string, "s");
	configuration.saturation.type = f32;
	configuration.saturation.ui_type = ui_type_slider;
	configuration.saturation.value.f32   = preferences.getFloat(configuration.saturation.name, 0.85);

	// Background
	strcpy(configuration.background.name, "background");
	strcpy(configuration.background.pretty_name, "Background");
	strcpy(configuration.background.type_string, "f32");
	strcpy(configuration.background.ui_type_string, "s");
	configuration.background.type = f32;
	configuration.background.ui_type = ui_type_slider;
	configuration.background.value.f32   = preferences.getFloat(configuration.background.name, 0.25);

	// Current Mode
	strcpy(configuration.current_mode.name, "current_mode");
	strcpy(configuration.current_mode.pretty_name, "Current Mode");
	strcpy(configuration.current_mode.type_string, "u32");
	strcpy(configuration.current_mode.ui_type_string, "n");
	configuration.current_mode.type = u32;
	configuration.current_mode.ui_type = ui_type_none;
	configuration.current_mode.value.u32   = (uint32_t)preferences.getInt(configuration.current_mode.name, 1);

	// Mirror Mode
	strcpy(configuration.mirror_mode.name, "mirror_mode");
	strcpy(configuration.mirror_mode.pretty_name, "Mirror Mode");
	strcpy(configuration.mirror_mode.type_string, "u32");
	strcpy(configuration.mirror_mode.ui_type_string, "t");
	configuration.mirror_mode.type = u32;
	configuration.mirror_mode.ui_type = ui_type_toggle;
	configuration.mirror_mode.value.u32   = (uint32_t)preferences.getInt(configuration.mirror_mode.name, 1);

	// Screensaver
	strcpy(configuration.screensaver.name, "screensaver");
	strcpy(configuration.screensaver.pretty_name, "Screensaver");
	strcpy(configuration.screensaver.type_string, "u32");
	strcpy(configuration.screensaver.ui_type_string, "mt");
	configuration.screensaver.type = u32;
	configuration.screensaver.ui_type = ui_type_menu_toggle;
	configuration.screensaver.value.u32   = (uint32_t)preferences.getInt(configuration.screensaver.name, 1);

	// Temporal Dithering
	strcpy(configuration.temporal_dithering.name, "dithering");
	strcpy(configuration.temporal_dithering.pretty_name, "Temporal Dithering");
	strcpy(configuration.temporal_dithering.type_string, "u32");
	strcpy(configuration.temporal_dithering.ui_type_string, "mt");
	configuration.temporal_dithering.type = u32;
	configuration.temporal_dithering.ui_type = ui_type_menu_toggle;
	configuration.temporal_dithering.value.u32   = (uint32_t)preferences.getInt(configuration.temporal_dithering.name, 1);
	
	// Reverse Color
	strcpy(configuration.reverse_color_range.name, "reverse_color");
	strcpy(configuration.reverse_color_range.pretty_name, "Reverse Color Range");
	strcpy(configuration.reverse_color_range.type_string, "u32");
	strcpy(configuration.reverse_color_range.ui_type_string, "t");
	configuration.reverse_color_range.type = u32;
	configuration.reverse_color_range.ui_type = ui_type_toggle;
	configuration.reverse_color_range.value.u32   = (uint32_t)preferences.getInt(configuration.reverse_color_range.name, 0);

	// Auto Color Cycling
	strcpy(configuration.auto_color_cycle.name, "auto_color");
	strcpy(configuration.auto_color_cycle.pretty_name, "Auto Color Cycle");
	strcpy(configuration.auto_color_cycle.type_string, "u32");
	strcpy(configuration.auto_color_cycle.ui_type_string, "t");
	configuration.auto_color_cycle.type = u32;
	configuration.auto_color_cycle.ui_type = ui_type_toggle;
	configuration.auto_color_cycle.value.u32   = (uint32_t)preferences.getInt(configuration.auto_color_cycle.name, 0);

	// Blur
	strcpy(configuration.blur.name, "blur");
	strcpy(configuration.blur.pretty_name, "Blur");	
	strcpy(configuration.blur.type_string, "f32");
	strcpy(configuration.blur.ui_type_string, "s");
	configuration.blur.type = f32;
	configuration.blur.ui_type = ui_type_slider;
	configuration.blur.value.f32 = preferences.getFloat(configuration.blur.name, 0.00);

	// Show Interface
	strcpy(configuration.show_ui.name, "show_ui");
	strcpy(configuration.show_ui.pretty_name, "Show Interface");
	strcpy(configuration.show_ui.type_string, "u32");
	strcpy(configuration.show_ui.ui_type_string, "mt");
	configuration.show_ui.type = u32;
	configuration.show_ui.ui_type = ui_type_menu_toggle;
	configuration.show_ui.value.u32   = (uint32_t)preferences.getInt(configuration.show_ui.name, 1);
}

// Save configuration to LittleFS
bool save_config() {
	config_item* config_location = reinterpret_cast<config_item*>(&configuration);
	uint16_t num_config_items = sizeof(configuration) / sizeof(config_item);

	for(uint16_t i = 0; i < num_config_items; i++){
		config_item item = *(config_location + i);
		type_t type = item.type;

		if(type == u32){
			//printf("Saving u32 %s: %d\n", item.name, item.value.u32);
			preferences.putInt(item.name, item.value.u32);
		} else if(type == i32){
			//printf("Saving i32 %s: %d\n", item.name, item.value.i32);
			preferences.putInt(item.name, item.value.i32);
		} else if(type == f32){
			//printf("Saving f32 %s: %f\n", item.name, item.value.f32);
			preferences.putFloat(item.name, item.value.f32);
		} else {
			printf("ERROR: Unknown type in save_config\n");
			return false;
		}
	}

	return true;
}

void save_config_delayed() {
	configuration_changed = true;
	last_save_request_ms = t_now_ms;
	save_request_open = true;
}

void sync_configuration_to_file_system() {
	if (save_request_open == true) {
		if ((t_now_ms - last_save_request_ms) >= MIN_SAVE_WAIT_MS) {
			filesystem_ready = false;
			printf("SAVING\n");
			save_config();
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

	// Attempt to load wifi creds from flash
	printf("LOADING NETWORK...");
	load_network_credentials();
}
