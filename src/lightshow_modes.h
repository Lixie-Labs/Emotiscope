// -----------------------------------------------------------------------------------------------------------------------------
//   _   _           _       _           _                                                           _                    _
//  | | (_)         | |     | |         | |                                                         | |                  | |
//  | |  _    __ _  | |__   | |_   ___  | |__     ___   __      __           _ __ ___     ___     __| |   ___   ___      | |__
//  | | | |  / _` | | '_ \  | __| / __| | '_ \   / _ \  \ \ /\ / /          | '_ ` _ \   / _ \   / _` |  / _ \ / __|     | '_ \ 
//  | | | | | (_| | | | | | | |_  \__ \ | | | | | (_) |  \ V  V /           | | | | | | | (_) | | (_| | |  __/ \__ \  _  | | | |
//  |_| |_|  \__, | |_| |_|  \__| |___/ |_| |_|  \___/    \_/\_/            |_| |_| |_|  \___/   \__,_|  \___| |___/ (_) |_| |_|
//            __/ |                                                 ______
//           |___/                                                 |______|
//
// Functions for outputting computed data in beautiful fashion to the LEDs based on which mode is selected

// The individual drawing functions for each mode are defined in these files:
#include "lightshow_modes/spectrum.h"
#include "lightshow_modes/octave.h"
#include "lightshow_modes/metronome.h"
#include "lightshow_modes/spectronome.h"
#include "lightshow_modes/hype.h"
#include "lightshow_modes/plot.h"
#include "lightshow_modes/bloom.h"
#include "lightshow_modes/analog.h"
#include "lightshow_modes/waveform.h"
#include "lightshow_modes/neutral.h"

#include "lightshow_modes/debug.h"
#include "lightshow_modes/presets.h"

lightshow_mode lightshow_modes[] = {
	{ "Analog",          &draw_analog        }, // 0
	{ "Spectrum",        &draw_spectrum      }, // 1
	{ "Octave",          &draw_octave        }, // 2
	{ "Metronome",       &draw_metronome     }, // 3
	{ "Spectronome",     &draw_spectronome   }, // 4
	{ "Hype",            &draw_hype          }, // 5
	{ "Bloom",           &draw_bloom         }, // 6
	{ "Neutral",         &draw_neutral       }, // 7

	{ "debug",           &draw_debug         }, // 8
	{ "presets",         &draw_presets       }, // 9
};

const uint16_t NUM_LIGHTSHOW_MODES = sizeof(lightshow_modes) / sizeof(lightshow_mode);

extern float lpf_drag; // Used for fade transition

// Lightshow Modes can be summoned by their string name shown in the UI
// This string is compared to the lightshow_modes[] table to derive a
// pointer to that mode's drawing function. Then, transition_to_new_mode()
// handles the switch
int16_t set_lightshow_mode_by_name(char* name){
	int16_t mode_index = -1;

	uint16_t num_modes = sizeof(lightshow_modes) / sizeof(lightshow_mode);
	for(uint16_t i = 0; i < num_modes; i++){
		if( strcmp(name, lightshow_modes[i].name) == 0 ){

			// "presets" is a special mode that is not a real mode, but an animation that plays when the presets menu is open
			if( strcmp(lightshow_modes[i].name, "presets") == 0 ){
				// DO EXTRA STEPS FOR PRESETS MENU MODE HERE
			}

			// Found a matching mode
			configuration.current_mode = i;

			lpf_drag = 1.0; // Causes slow fade using low pass filtered image
			mode_index = i;
			break;
		}
	}

	return mode_index;
}

void increment_mode(){
	int16_t new_mode = configuration.current_mode + 1;
	configuration.current_mode = new_mode % NUM_LIGHTSHOW_MODES;
	lpf_drag = 1.0; // Causes slow fade using low pass filtered image
	save_config_delayed();
}









/*

void draw_tempi() {
	uint16_t profiler_index = start_function_timing(__func__);
	memset(leds, 0, sizeof(CRGBF) * NUM_LEDS);

	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float tempi_magnitude = tempi[tempo_bin].magnitude;
		CRGBF tempi_color = {0.0, tempi_magnitude * tempi_magnitude, 0.0};
		leds[tempo_bin] = tempi_color;
	}

	end_function_timing(profiler_index);
}

void draw_novelty_curve() {
	uint16_t profiler_index = start_function_timing(__func__);
	memset(leds, 0, sizeof(CRGBF) * NUM_LEDS);

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		float value = novelty_curve_normalized[((NOVELTY_HISTORY_LENGTH - 1) - NUM_LEDS) + i];

		CRGBF novelty_color = {value * value, 0.0, 0.0};
		leds[i] = novelty_color;
	}

	end_function_timing(profiler_index);
}

void draw_novelty_curve_full() {
	uint16_t profiler_index = start_function_timing(__func__);
	memset(leds, 0, sizeof(CRGBF) * NUM_LEDS);

	uint16_t multiple = NOVELTY_HISTORY_LENGTH / NUM_LEDS;

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		float max_val = 0.0;
		for (uint16_t m = 0; m < multiple; m++) {
			float value = novelty_curve_normalized[i * multiple + m];
			max_val = max(max_val, value);
		}

		CRGBF novelty_color = {max_val * max_val, 0.0, 0.0};
		leds[i] = novelty_color;
	}

	end_function_timing(profiler_index);
}

void draw_waveform(bool add_mode) {
	uint16_t profiler_index = start_function_timing(__func__);
	const uint8_t NUM_AVERAGE_SAMPLES = 16;
	static float smoothed_waveform[NUM_LEDS];

	if (add_mode == false) {
		memset(leds, 0, sizeof(CRGBF) * NUM_LEDS);	// Clear the display
	}

	if (!waveform_locked) {
		float max_val = 0.000001;
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float sample_sum = 0.0;
			for (uint8_t a = 0; a < NUM_AVERAGE_SAMPLES; a++) {
				uint16_t index = ((8192 - 1) - (NUM_LEDS * NUM_AVERAGE_SAMPLES)) + (NUM_LEDS * a) + i;
				float sample = clip_float(sample_history[index]);
				sample_sum += sample;
			}

			smoothed_waveform[i] = sample_sum / float(NUM_AVERAGE_SAMPLES);
			max_val = max(max_val, smoothed_waveform[i]);
		}

		max_val = max(max_val, 0.1f);

		float auto_scale = 1.0 / max_val;

		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			smoothed_waveform[i] *= auto_scale;
		}
	}

	for (uint16_t i = 0; i < NUM_LEDS >> 1; i++) {
		float sample = smoothed_waveform[i];
		sample = sample * sample * sample;
		sample = sample * 0.98 + 0.02;

		leds[64 + i] = {0.0, sample, 0.0};
		leds[63 - i] = {0.0, sample, 0.0};
	}

	end_function_timing(profiler_index);
}

*/