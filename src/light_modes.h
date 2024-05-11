/*
-----------------------------------------------------------------------------------------------------------------------------
  _   _           _       _           _                                                           _                    _
 | | (_)         | |     | |         | |                                                         | |                  | |
 | |  _    __ _  | |__   | |_   ___  | |__     ___   __      __           _ __ ___     ___     __| |   ___   ___      | |__
 | | | |  / _` | | '_ \  | __| / __| | '_ \   / _ \  \ \ /\ / /          | '_ ` _ \   / _ \   / _` |  / _ \ / __|     | '_ \ 
 | | | | | (_| | | | | | | |_  \__ \ | | | | | (_) |  \ V  V /           | | | | | | | (_) | | (_| | |  __/ \__ \  _  | | | |
 |_| |_|  \__, | |_| |_|  \__| |___/ |_| |_|  \___/    \_/\_/            |_| |_| |_|  \___/   \__,_|  \___| |___/ (_) |_| |_|
           __/ |                                                 ______
          |___/                                                 |______|

Functions for outputting computed data in beautiful fashion to the LEDs based on which mode is selected
*/

// The individual drawing functions for each mode are defined in these files:

// ACTIVE MODES
#include "light_modes/active/analog.h"
#include "light_modes/active/spectrum.h"
#include "light_modes/active/octave.h"
#include "light_modes/active/metronome.h"
#include "light_modes/active/spectronome.h"
#include "light_modes/active/hype.h"
#include "light_modes/active/bloom.h"

// INACTIVE MODES
#include "light_modes/inactive/neutral.h"

// SYSTEM MODES
#include "light_modes/system/self_test.h"
#include "light_modes/system/presets.h"

uint16_t NUM_LIGHT_MODES = 0;
int16_t queued_light_mode_index = 0;

light_mode light_modes[] = {
	// Active Modes
	{ "Analog",          LIGHT_MODE_TYPE_ACTIVE,    &draw_analog        },
	{ "Spectrum",        LIGHT_MODE_TYPE_ACTIVE,    &draw_spectrum      },
	{ "Octave",          LIGHT_MODE_TYPE_ACTIVE,    &draw_octave        },
	{ "Metronome",       LIGHT_MODE_TYPE_ACTIVE,    &draw_metronome     },
	{ "Spectronome",     LIGHT_MODE_TYPE_ACTIVE,    &draw_spectronome   },
	{ "Hype",            LIGHT_MODE_TYPE_ACTIVE,    &draw_hype          },
	{ "Bloom",           LIGHT_MODE_TYPE_ACTIVE,    &draw_bloom         },

	// Inactive Modes
	{ "Neutral",         LIGHT_MODE_TYPE_INACTIVE,  &draw_neutral       },

	// System Modes
	{ "Self Test",       LIGHT_MODE_TYPE_SYSTEM,    &draw_self_test     },

	//{ "debug",           &draw_debug         }, // 8
	//{ "presets",         &draw_presets       }, // 9
};

extern float lpf_drag; // Used for fade transition

void init_light_mode_list(){
	uint16_t num_light_modes_raw = sizeof(light_modes) / sizeof(light_mode);
	NUM_LIGHT_MODES = 0;

	// Count the non-system light modes
	for(uint16_t i = 0; i < num_light_modes_raw; i++){
		if(light_modes[i].type != LIGHT_MODE_TYPE_SYSTEM){
			NUM_LIGHT_MODES++;
		}
	}
}

void set_light_mode_by_index(uint16_t mode_index){
	configuration.current_mode = mode_index;
	lpf_drag = 1.0; // Causes slow fade using low pass filtered image
	save_config_delayed();
}

int16_t get_light_mode_index_by_name(const char* name){
	uint16_t num_modes = sizeof(light_modes) / sizeof(light_mode);
	for(uint16_t i = 0; i < num_modes; i++){
		if( strcmp(name, light_modes[i].name) == 0 ){
			// Found match
			return i;
		}
	}	

	// Found no matches?
	return -1;
}

// Light Modes can be summoned by their string name shown in the UI
// This string is compared to the light_modes[] table to derive a
// pointer to that mode's drawing function. Then, transition_to_new_mode()
// handles the switch
int16_t set_light_mode_by_name(const char* name){
	int16_t mode_index = get_light_mode_index_by_name(name);
	set_light_mode_by_index(mode_index);

	return mode_index;
}

int16_t increment_mode(){
	int16_t next_mode_index = (configuration.current_mode + 1) % NUM_LIGHT_MODES;
	set_light_mode_by_index(next_mode_index);

	return next_mode_index;
}

void enter_queued_light_mode(){
	set_light_mode_by_index(queued_light_mode_index);
}

void queue_light_mode_by_name(const char* name){
	queued_light_mode_index = get_light_mode_index_by_name(name);
}

void queue_light_mode_by_index(uint16_t mode_index){
	queued_light_mode_index = mode_index;
}

void trigger_self_test(){
	queue_light_mode_by_index( configuration.current_mode ); // Save current mode to return to
	self_test_step = SELF_TEST_STEP_START; // Set the self test to starting state
	set_light_mode_by_name("Self Test"); // Jump to self test mode
}