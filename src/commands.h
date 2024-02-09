#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_QUEUE_SLOTS (32)

command command_queue[COMMAND_QUEUE_SLOTS];
uint16_t commands_queued = 0;

extern float clip_float(float input);
extern int16_t set_lightshow_mode_by_name(char* name);
extern void transmit_to_client_in_slot(char* message, uint8_t client_slot);

// Function to return the selected index as a null-terminated string with custom delimiter
// The result is stored in the provided buffer
bool load_substring_from_split_index(const char* input, int index, char* result, size_t result_size, char delimiter = '|') {
    // Ensure input and result buffer are not NULL
    if (input == NULL || result == NULL) {
        return false;
    }

    int len = strlen(input);
    int start = 0;
    int end = 0;
    int current_index = 0;

    // Iterate over the input to find the desired index
    for (int i = 0; i <= len; i++) {
        // Check for custom delimiter or end of string
        if (input[i] == delimiter || input[i] == '\0') {
            if (current_index == index) {
                end = i;
                break;
            }
            start = i + 1;
            current_index++;
        }
    }

    // Check if the index was not found or if the result buffer is too small
    int segment_length = end - start;
    if (current_index != index || segment_length + 1 > result_size) {
        return false;
    }

    // Copy the substring to the result buffer
	memset(result, 0, result_size);
    strncpy(result, input + start, segment_length);
    result[segment_length] = '\0';

    return true;
}

void shift_command_queue_left() {
	memmove(command_queue, command_queue + 1, (COMMAND_QUEUE_SLOTS - 1) * (sizeof(command)));
	memset(command_queue + COMMAND_QUEUE_SLOTS - 1, 0, 1 * sizeof(command));
}

void unrecognized_command_error(char* command){
	printf("UNRECOGNIZED COMMAND: %s\n", command);
}

void parse_command(command com) {
	// printf("Parsing command: '%s'\n", com.command);
	// Buffer to store results from get_index
    char substring[MAX_COMMAND_LENGTH];

	// Get command type
	load_substring_from_split_index(com.command, 0, substring, sizeof(substring));

	if (fastcmp(substring, "set")) {
		// Get setting name
		load_substring_from_split_index(com.command, 1, substring, sizeof(substring));
		if (fastcmp(substring, "brightness")) {
			// Get brightness value
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));
			float setting_value = clip_float(atof(substring));
			configuration.brightness = setting_value;
			rendered_debug_value = configuration.brightness;
		}
		else if (fastcmp(substring, "speed")) {
			// Get speed value
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));
			float setting_value = atof(substring);
			configuration.speed = setting_value;

			rendered_debug_value = (configuration.speed-0.5) / (10.0-0.5);
		}
		else if (fastcmp(substring, "hue")) {
			// Get brightness value
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));
			float setting_value = clip_float(atof(substring));
			configuration.hue = setting_value;
			rendered_debug_value = configuration.hue; // TODO: Color-related changes shouldn't show a UI dot
		}
		
		else if (fastcmp(substring, "mirror_mode")) {
			// Get mirror_mode value
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));
			bool setting_value = (bool)atoi(substring);
			configuration.mirror_mode = setting_value;
		}
		else if (fastcmp(substring, "incandescent")) {
			// Get incandescent_filter value
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));
			float setting_value = atof(substring);
			configuration.incandescent_filter = setting_value;
		}
		else if (fastcmp(substring, "hue_range")) {
			// Get hue_range value
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));
			float setting_value = atof(substring);
			configuration.hue_range = setting_value; // -1.0 to 1.0 range
		}

		else if (fastcmp(substring, "mode")) {
			// Get mode name
			load_substring_from_split_index(com.command, 2, substring, sizeof(substring));

			int16_t mode_index = set_lightshow_mode_by_name(substring);
			if(mode_index == -1){
				unrecognized_command_error(substring);
			}
			else{
				load_sliders_relevant_to_mode(mode_index);
				load_toggles_relevant_to_mode(mode_index);
				transmit_to_client_in_slot("mode_selected", com.origin_client_slot);
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
		load_substring_from_split_index(com.command, 1, substring, sizeof(substring));

		// If getting configuration struct contents
		if (fastcmp(substring, "config")) {
			sync_configuration_to_client();
		}

		// If getting mode list
		else if (fastcmp(substring, "modes")) {
			transmit_to_client_in_slot("clear_modes", com.origin_client_slot);

			uint16_t num_modes = sizeof(lightshow_modes) / sizeof(lightshow_mode);
			for(uint16_t i = 0; i < num_modes; i++){
				char command_string[40];
				snprintf(command_string, 40, "new_mode|%s", lightshow_modes[i].name);
				transmit_to_client_in_slot(command_string, com.origin_client_slot);
			}

			transmit_to_client_in_slot("modes_ready", com.origin_client_slot);
		}
		// If getting slider list
		else if (fastcmp(substring, "sliders")) {
			transmit_to_client_in_slot("clear_sliders", com.origin_client_slot);

			for(uint16_t i = 0; i < sliders_active; i++){
				char command_string[80];
				snprintf(command_string, 80, "new_slider|%s|%.3f|%.3f|%.3f", sliders[i].name, sliders[i].slider_min, sliders[i].slider_max, sliders[i].slider_step);
				transmit_to_client_in_slot(command_string, com.origin_client_slot);
			}

			transmit_to_client_in_slot("sliders_ready", com.origin_client_slot);
		}

		// If getting toggle list
		else if (fastcmp(substring, "toggles")) {
			transmit_to_client_in_slot("clear_toggles", com.origin_client_slot);

			for(uint16_t i = 0; i < toggles_active; i++){
				char command_string[80];
				snprintf(command_string, 80, "new_toggle|%s", toggles[i].name);
				transmit_to_client_in_slot(command_string, com.origin_client_slot);
			}

			transmit_to_client_in_slot("toggles_ready", com.origin_client_slot);
		}

		// Couldn't figure out what to "get"
		else{
			unrecognized_command_error(substring);
		}
	}
	else if (fastcmp(substring, "reset")) {
		transmit_to_client_in_slot("disconnect_immediately", com.origin_client_slot);
		printf("Device was instructed to soft-reset! Please wait...\n");
		delay(100);
		ESP.restart();
	}
	else if (fastcmp(substring, "noise_cal")) {
		start_noise_calibration();
	}
	else if (fastcmp(substring, "ping")) {
		transmit_to_client_in_slot("pong", com.origin_client_slot);
	}
	else if (fastcmp(substring, "start_debug_recording")) {
		audio_recording_index = 0;
		memset(audio_debug_recording, 0, sizeof(int16_t)*MAX_AUDIO_RECORDING_SAMPLES);
		audio_recording_live = true;
	}
	else{
		unrecognized_command_error(substring);
	}

	// printf("current brightness value: %.3f\n", configuration.brightness);
}

void process_command_queue() {
	if (commands_queued > 0) {
		parse_command(command_queue[0]);
		shift_command_queue_left();
		commands_queued -= 1;
	}
}

bool queue_command(char* command, uint8_t length, uint8_t client_slot) {
	if (length < MAX_COMMAND_LENGTH) {
		if (commands_queued < COMMAND_QUEUE_SLOTS - 1) {
			memcpy(command_queue[commands_queued].command, command, length);
			command_queue[commands_queued].origin_client_slot = client_slot;
			commands_queued += 1;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}