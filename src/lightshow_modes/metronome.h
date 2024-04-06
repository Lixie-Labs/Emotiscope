// Function to shape a linear input to a positive half sine wave
float linear_to_half_sine(float x) {
    // Ensure the input is clamped between 0.0 and 1.0
    if (x < 0.0) x = 0.0;
    if (x > 1.0) x = 1.0;

    // Scale the input from [0, 1] to [0, PI] for a half sine wave
    float scaled_input = x * M_PI;

    // Compute the sine, which will be in the range [-1, 1]
    // No need to adjust the output range because we're only interested in the positive half
    float half_sine_output = sin(scaled_input);

    return half_sine_output;
}

float linear_to_half_sine_hold(float x) {
    // Return 0.0 for inputs outside the range [0.25, 0.75]
    if (x < 0.25 || x > 0.75) {
        return 0.0;
    }

    // Normalize the input from [0.25, 0.75] to [0, 1] for sine computation
    float normalized_input = (x - 0.25) / 0.5;

    // Scale the normalized input from [0, 1] to [0, PI] for a half sine wave
    float scaled_input = normalized_input * M_PI;

    // Compute the sine for the scaled input
    float half_sine_output = sin(scaled_input);

    return half_sine_output;
}

void draw_metronome() {
	static uint32_t iter = 0;
	iter++;

	// -------------------------------------------
	// Draw tempi to the display
	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		/*
		if (debug == true) {
			float progress = float(tempo_bin) / NUM_TEMPI;
			// Draw tempi and novelty curve together
			float tempi_val = tempi_smooth[tempo_bin];

			float novelty_val = novelty_curve_normalized[((NOVELTY_HISTORY_LENGTH - 1) - NUM_TEMPI) + tempo_bin];  // Pulls {NUM_TEMPI} of the most recent novelty samples
			novelty_val *= novelty_val;
			novelty_val *= 0.5;

			float beat = 1.0 - (tempi[tempo_bin].beat * 0.5 + 0.5);
			beat = beat * beat * beat * beat * (tempi_val * tempi_val);

			beat = beat * 0.99 + 0.01;

			CRGBF novelty_color = {novelty_val, 0.0, 0.0};
			CRGBF tempi_color = hsv(progress, 0.9, beat);

			// leds[tempo_bin] = novelty_color;
			leds[tempo_bin] = add(leds[tempo_bin], tempi_color);
		}
		*/
	}

	// For debug printing purposes ---------------------------------------------------
	/*
	if (debug == true) {
		float search_bpm = 80.0;
		uint16_t tempo_bin = find_closest_tempo_bin(search_bpm);

		if (iter % 5 == 0) {
			// Serial.println(tempi[tempo_bin].beat);
		}
	}
	*/
	// For debug printing purposes ---------------------------------------------------

	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float progress = float(tempo_bin) / NUM_TEMPI;
		float tempi_magnitude = tempi_smooth[tempo_bin];

		float contribution = (tempi_magnitude / tempi_power_sum) * tempi_magnitude;

		/*
		float phase = (tempi[tempo_bin].phase + M_PI) / (2.0 * M_PI);

		phase = fmod(phase+0.75, 1.0);

		phase = linear_to_half_sine_hold(phase);
		*/

		float sine = sin(tempi[tempo_bin].phase + (PI*0.5));
		sine *= 2.0;

		if(sine > 1.0){ sine = 1.0; }
		else if(sine < -1.0){ sine = -1.0; }

		float metronome_width = 0.5; // Too wide of a show can be distracting, 50% is enough for the effect
		float dot_pos = clip_float( sine * (0.5*(sqrt(contribution)) * metronome_width) + 0.5 );

		float opacity = (sqrt(contribution));

		float color_offset = 0.0;
		if(tempo_bin % 2 == 0){
			color_offset = 0.25;
		}

		if(opacity > 0.0001){
			CRGBF dot_color = hsv((configuration.color+color_offset*configuration.color_range) + configuration.color_range*progress, configuration.saturation, 1.0);

			if(configuration.mirror_mode == true){
				dot_pos -= 0.25;
			}

			draw_dot(leds, NUM_RESERVED_DOTS + tempo_bin * 2 + 0, dot_color, dot_pos, opacity);

			if(configuration.mirror_mode == true){
				draw_dot(leds, NUM_RESERVED_DOTS + tempo_bin * 2 + 1, dot_color, 1.0 - dot_pos, opacity);
			}
		}
	}
}