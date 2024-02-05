void draw_spectrum() {
	uint16_t profiler_index = start_function_timing(__func__);

	float magnitudes[NUM_FREQS];
	for (uint16_t i = 0; i < NUM_FREQS; i += 4) {
		magnitudes[i + 0] = frequencies_musical[i + 0].magnitude;
		magnitudes[i + 1] = frequencies_musical[i + 1].magnitude;
		magnitudes[i + 2] = frequencies_musical[i + 2].magnitude;
		magnitudes[i + 3] = frequencies_musical[i + 3].magnitude;
	}

	if(configuration.mirror_mode == true){ // Mirror mode
		for (uint16_t i = 0; i < NUM_LEDS>>1; i++) {
			float progress = float(i) / (NUM_LEDS>>1);
			float mag = magnitudes[i];
			CRGBF color = hsv(configuration.hue+(progress*configuration.hue_range), 1.0, mag);
			
			leds[63-i] = color;
			leds[64+i] = color;
		}
	}
	else{ // Non mirror
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = float(i) / NUM_LEDS;
			float mag = clip_float(interpolate(progress, magnitudes, NUM_FREQS));
			CRGBF color = hsv(configuration.hue+(progress*configuration.hue_range), 1.0, mag);

			leds[i] = hsv(configuration.hue+(progress*configuration.hue_range), 1.0, mag);
		}
	}

	end_function_timing(profiler_index);
}