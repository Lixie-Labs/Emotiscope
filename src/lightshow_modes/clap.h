void draw_clap() {
	uint16_t profiler_index = start_function_timing(__func__);

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

		float dot_pos = clip_float(sqrt((tempi[tempo_bin].beat * 0.5 + 0.5)) * sqrt(sqrt(contribution)));
		float opacity = contribution;
		// opacity *= opacity;

		CRGBF dot_color = hsv(progress, 0.9, 1.0);
		if (tempo_bin % 2 == 0) {
			//dot_color = hsv(progress+0.5, 0.9, 1.0);
		}

		// if (debug == true) {
		//	draw_dot(leds, tempo_bin * 2 + 0, dot_color, (dot_pos) * 0.5 + 0.5, opacity);
		//	draw_dot(leds, tempo_bin * 2 + 1, dot_color, (1.0 - dot_pos) * 0.5 + 0.5, opacity);
		// }
		// else {

		if(configuration.mirror_mode == true){
			dot_pos = dot_pos * 0.5 + 0.5;	// scale to half
		}

		draw_dot(leds, tempo_bin * 2 + 0, dot_color, dot_pos, opacity);

		if(configuration.mirror_mode == true){
			draw_dot(leds, tempo_bin * 2 + 1, dot_color, 1.0 - dot_pos, opacity);
		}
		//}
	}

	end_function_timing(profiler_index);
}