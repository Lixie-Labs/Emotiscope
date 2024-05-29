#define NETWORK_CONFIG_FILENAME "/secrets/network.txt"
#define MIN_SAVE_WAIT_MS (3 * 1000)	 // Values must stabilize for this many seconds to be written to NVS

Preferences preferences; // NVS storage for configuration

config configuration; // configuration struct to be filled by NVS or defaults on boot

extern light_mode light_modes[];
extern PsychicWebSocketHandler websocket_handler;

volatile bool wifi_config_mode = false;

volatile uint32_t last_save_request_ms = 0;
volatile bool save_request_open = false;

volatile bool filesystem_ready = true;

void load_config(){
	// Load configuration from NVS

	memset(&configuration, 0, sizeof(config)); // Clear the configuration struct

	// Brightness
	strcpy(configuration.brightness.name, "brightness");
	strcpy(configuration.brightness.pretty_name, "Brightness");	
	configuration.brightness.value.f32 = preferences.getFloat(configuration.brightness.name, 1.00);

	// Softness
	strcpy(configuration.softness.name, "softness");
	strcpy(configuration.softness.pretty_name, "Softness");
	configuration.softness.value.f32   = preferences.getFloat(configuration.softness.name, 0.25);

	// Color
	strcpy(configuration.color.name, "color");
	strcpy(configuration.color.pretty_name, "Color");
	configuration.color.value.f32   = preferences.getFloat(configuration.color.name, 0.33);

	// Color Range
	strcpy(configuration.color_range.name, "color_range");
	strcpy(configuration.color_range.pretty_name, "Color Range");
	configuration.color_range.value.f32   = preferences.getFloat(configuration.color_range.name, 0.0);

	// Warmth
	strcpy(configuration.warmth.name, "warmth");
	strcpy(configuration.warmth.pretty_name, "Warmth");
	configuration.warmth.value.f32   = preferences.getFloat(configuration.warmth.name, 0.50);

	// Speed
	strcpy(configuration.speed.name, "speed");
	strcpy(configuration.speed.pretty_name, "Speed");
	configuration.speed.value.f32   = preferences.getFloat(configuration.speed.name, 0.50);

	// Saturation
	strcpy(configuration.saturation.name, "saturation");
	strcpy(configuration.saturation.pretty_name, "Saturation");
	configuration.saturation.value.f32   = preferences.getFloat(configuration.saturation.name, 0.85);

	// Background
	strcpy(configuration.background.name, "background");
	strcpy(configuration.background.pretty_name, "Background");
	configuration.background.value.f32   = preferences.getFloat(configuration.background.name, 0.25);

	// Current Mode
	strcpy(configuration.current_mode.name, "current_mode");
	strcpy(configuration.current_mode.pretty_name, "Current Mode");
	configuration.current_mode.value.u32   = (uint32_t)preferences.getInt(configuration.current_mode.name, 1);

	// Mirror Mode
	strcpy(configuration.mirror_mode.name, "mirror_mode");
	strcpy(configuration.mirror_mode.pretty_name, "Mirror Mode");
	configuration.mirror_mode.value.u32   = (uint32_t)preferences.getBool(configuration.mirror_mode.name, true);

	// Screensaver
	strcpy(configuration.screensaver.name, "screensaver");
	strcpy(configuration.screensaver.pretty_name, "Screensaver");
	configuration.screensaver.value.u32   = (uint32_t)preferences.getBool(configuration.screensaver.name, true);

	// Temporal Dithering
	strcpy(configuration.temporal_dithering.name, "dithering");
	strcpy(configuration.temporal_dithering.pretty_name, "Temporal Dithering");
	configuration.temporal_dithering.value.u32   = (uint32_t)preferences.getBool(configuration.temporal_dithering.name, true);
	
	// Reverse Color
	strcpy(configuration.reverse_color_range.name, "reverse_color");
	strcpy(configuration.reverse_color_range.pretty_name, "Reverse Color Range");
	configuration.reverse_color_range.value.u32   = (uint32_t)preferences.getBool(configuration.reverse_color_range.name, false);

	// Auto Color Cycling
	strcpy(configuration.auto_color_cycle.name, "auto_color");
	strcpy(configuration.auto_color_cycle.pretty_name, "Auto Color Cycle");
	configuration.auto_color_cycle.value.u32   = (uint32_t)preferences.getBool(configuration.auto_color_cycle.name, false);

	// Blur
	strcpy(configuration.blur.name, "blur");
	strcpy(configuration.blur.pretty_name, "Blur");	
	configuration.blur.value.f32 = preferences.getFloat(configuration.blur.name, 0.00);

	// Show Interface
	strcpy(configuration.show_ui.name, "show_ui");
	strcpy(configuration.show_ui.pretty_name, "Show Interface");
	configuration.show_ui.value.u32   = (uint32_t)preferences.getBool(configuration.show_ui.name, true);
}

// Save configuration to LittleFS
bool save_config() {
	preferences.putFloat(configuration.brightness.name,         configuration.brightness.value.f32);
	preferences.putFloat(configuration.softness.name,           configuration.softness.value.f32);
	preferences.putFloat(configuration.color.name,              configuration.color.value.f32);
	preferences.putFloat(configuration.color_range.name,        configuration.color_range.value.f32);
	preferences.putFloat(configuration.warmth.name,             configuration.warmth.value.f32);
	preferences.putFloat(configuration.speed.name,              configuration.speed.value.f32);
	preferences.putFloat(configuration.saturation.name,         configuration.saturation.value.f32);
	preferences.putFloat(configuration.background.name,         configuration.background.value.f32);
	preferences.putInt(configuration.current_mode.name,         configuration.current_mode.value.u32);
	preferences.putBool(configuration.mirror_mode.name,         configuration.mirror_mode.value.u32);
	preferences.putBool(configuration.screensaver.name,         configuration.screensaver.value.u32);
	preferences.putBool(configuration.temporal_dithering.name,  configuration.temporal_dithering.value.u32);
	preferences.putBool(configuration.reverse_color_range.name, configuration.reverse_color_range.value.u32);
	preferences.putBool(configuration.auto_color_cycle.name,    configuration.auto_color_cycle.value.u32);
	preferences.putFloat(configuration.blur.name,               configuration.blur.value.f32);
	preferences.putBool(configuration.show_ui.name,             configuration.show_ui.value.u32);

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
