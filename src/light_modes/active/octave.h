void draw_octave() {
	if(configuration.mirror_mode == true){ // Mirror mode
		for (uint16_t i = 0; i < (NUM_LEDS >> 1); i++) {
			float progress = num_leds_float_lookup[i<<1];
			float mag = clip_float(interpolate(progress, chromagram, 12));
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation,
				mag
			);

			leds[ (NUM_LEDS>>1)    + i] = color;
			leds[((NUM_LEDS>>1)-1) - i] = color;
		}
	}
	else{ // Non mirror
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = num_leds_float_lookup[i];
			float mag = clip_float(interpolate(progress, chromagram, 12));
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation,
				mag
			);

			leds[i] = color;
		}
	}
}