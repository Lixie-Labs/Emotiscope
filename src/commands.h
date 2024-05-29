#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern float clip_float(float input);
extern int16_t set_lightshow_mode_by_name(char* name);
extern void reboot_into_wifi_config_mode();


void unrecognized_command_error(char* command){
	printf("UNRECOGNIZED COMMAND: %s\n", command);
}

void parse_command(char* command, PsychicWebSocketRequest *request) {
	//printf("Parsing command: '%s'\n", command);
	
	fetch_substring(command, '|', 0);

	if (fastcmp(substring, "emotiscope?")){
		request->client()->sendMessage("emotiscope");
	}

	else if (fastcmp(substring, "set")) {
		// Get setting name
		fetch_substring(command, '|', 1);
		
		if (fastcmp(substring, "brightness")) {
			// Get brightness value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.brightness.value.f32 = setting_value;

			update_ui(UI_NEEDLE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "softness")) {
			// Get softness value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.softness.value.f32 = setting_value;

			update_ui(UI_NEEDLE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "speed")) {
			// Get speed value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.speed.value.f32 = setting_value;

			update_ui(UI_NEEDLE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "blur")) {
			// Get blur value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.blur.value.f32 = setting_value;

			update_ui(UI_NEEDLE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "color")) {
			// Get color value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.color.value.f32 = setting_value;

			update_ui(UI_HUE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "mirror_mode")) {
			// Get mirror_mode value
			fetch_substring(command, '|', 2);
			uint32_t setting_value = (bool)atoi(substring);
			configuration.mirror_mode.value.u32 = setting_value;
		}
		else if (fastcmp(substring, "warmth")) {
			// Get warmth value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.warmth.value.f32 = setting_value;

			update_ui(UI_NEEDLE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "color_range")) {
			// Get color_range value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.color_range.value.f32 = setting_value;

			update_ui(UI_HUE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "saturation")) {
			// Get saturation value
			fetch_substring(command, '|', 2);
			float setting_value = sqrt(sqrt(clip_float(atof(substring))));
			configuration.saturation.value.f32 = setting_value;

			update_ui(UI_HUE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "background")) {
			// Get background value
			fetch_substring(command, '|', 2);
			float setting_value = clip_float(atof(substring));
			configuration.background.value.f32 = setting_value;

			update_ui(UI_NEEDLE_EVENT, setting_value);
		}
		else if (fastcmp(substring, "screensaver")) {
			// Get screensaver value
			fetch_substring(command, '|', 2);
			uint32_t setting_value = (bool)atoi(substring);
			configuration.screensaver.value.u32 = setting_value;
		}
		else if (fastcmp(substring, "temporal_dithering")){
			// Get temporal_dithering value
			fetch_substring(command, '|', 2);
			uint32_t setting_value = (bool)atoi(substring);
			configuration.temporal_dithering.value.u32 = setting_value;
		}
		else if (fastcmp(substring, "reverse_color_range")){
			// Get reverse_color_range value
			fetch_substring(command, '|', 2);
			uint32_t setting_value = (bool)atoi(substring);
			configuration.reverse_color_range.value.u32 = setting_value;
		}
		else if (fastcmp(substring, "auto_color_cycle")){
			// Get auto_color_cycle value
			fetch_substring(command, '|', 2);
			uint32_t setting_value = (bool)atoi(substring);
			configuration.auto_color_cycle.value.u32 = setting_value;
		}
		else if (fastcmp(substring, "show_ui")){
			// Get show_interface value
			fetch_substring(command, '|', 2);
			uint32_t setting_value = (bool)atoi(substring);
			configuration.show_ui.value.u32 = setting_value;
		}

		else if (fastcmp(substring, "mode")) {
			// Get mode name
			fetch_substring(command, '|', 2);

			int16_t mode_index = set_light_mode_by_name(substring);
			if(mode_index == -1){
				unrecognized_command_error(substring);
			}
			else{
				load_sliders_relevant_to_mode(mode_index);
				load_toggles_relevant_to_mode(mode_index);
				broadcast("reload_config");
			}
		}
		else{
			unrecognized_command_error(substring);
		}

		// Open a save request for later
		save_config_delayed();
	}
	else if (fastcmp(substring, "get")) {
		// Name of thing to get
		fetch_substring(command, '|', 1);
		
		// If getting configuration struct contents
		if (fastcmp(substring, "config")) {
			// Wake on command
			EMOTISCOPE_ACTIVE = true;
			load_menu_toggles();
		}

		// If getting version number
		else if (fastcmp(substring, "version")) {
			char command_string[128];
			snprintf(command_string, 128, "version|%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH);
			request->client()->sendMessage(command_string);
		}

		// Couldn't figure out what to "get"
		else{
			unrecognized_command_error(substring);
		}
	}
	else if (fastcmp(substring, "reboot")) {
		request->client()->sendMessage("disconnect_immediately");
		printf("Device was instructed to reboot! Please wait...\n");
		delay(100);
		ESP.restart();
	}
	else if (fastcmp(substring, "reboot_wifi_config")) {
		request->client()->sendMessage("disconnect_immediately");
		printf("Device was instructed to reboot into WiFi config mode! Please wait...\n");
		reboot_into_wifi_config_mode();
	}
	else if (fastcmp(substring, "button_tap")) {
		printf("REMOTE TAP TRIGGER\n");
		if(EMOTISCOPE_ACTIVE == true){
			int16_t next_mode_index = increment_mode();
			load_sliders_relevant_to_mode(next_mode_index);
			load_toggles_relevant_to_mode(next_mode_index);
			broadcast("reload_config");
		}
		else{
			toggle_standby();
		}
	}
	else if (fastcmp(substring, "button_hold")) {
		printf("REMOTE HOLD TRIGGER\n");
		toggle_standby();
	}
	else if (fastcmp(substring, "ping")) {
		request->client()->sendMessage("pong");
	}
	else if (fastcmp(substring, "touch_start")) {
		printf("APP TOUCH START\n");
		app_touch_active = true;
	}
	else if (fastcmp(substring, "touch_end")) {
		printf("APP TOUCH END\n");
		app_touch_active = false;
	}
	else if (fastcmp(substring, "slider_touch_start")) {
		slider_touch_active = true;
	}
	else if (fastcmp(substring, "slider_touch_end")) {
		slider_touch_active = false;
	}
	else if (fastcmp(substring, "check_update")) {
		extern bool check_update();
		if(check_update() == true){ // Update available
			request->client()->sendMessage("update_available");
		}
		else{
			request->client()->sendMessage("no_updates");
		}
	}
	else if (fastcmp(substring, "perform_update")) {
		extern void perform_update(PsychicWebSocketRequest *request);
		perform_update(request);
	}
	else if (fastcmp(substring, "self_test")) {
		if(t_now_ms >= 1000){ // Wait 1 second before checking boot button
			if(self_test_step == SELF_TEST_INACTIVE){ // Self test is not already running
				printf("SELF TEST TRIGGERED FROM APP, BEGINNING SELF TEST\n");
				EMOTISCOPE_ACTIVE = true; // Wake if needed
				trigger_self_test(); // Begin self test
			}
		}
	}
	else if(fastcmp(substring, "increment_mode")){
		int16_t next_mode_index = increment_mode();
		load_sliders_relevant_to_mode(next_mode_index);
		load_toggles_relevant_to_mode(next_mode_index);
		broadcast("reload_config");
	}

	else{
		unrecognized_command_error(substring);
	}

	// printf("current brightness value: %.3f\n", configuration.brightness);
}