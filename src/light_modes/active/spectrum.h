void draw_spectrum() {
	// Mirror mode
	if(configuration.mirror_mode.value.u32 == true){
		for (uint16_t i = 0; i < NUM_LEDS>>1; i++) {
			float progress = num_leds_float_lookup[i<<1];
			float mag = (spectrogram_smooth[i]);
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				mag
			);

			leds[ (NUM_LEDS>>1)    + i] = color;
			leds[((NUM_LEDS>>1)-1) - i] = color;
		}
	}
	// Non mirror
	else{ 
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = num_leds_float_lookup[i];
			float mag = (clip_float(interpolate(progress, spectrogram_smooth, NUM_FREQS)));
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				mag
			);

			leds[i] = color;
		}
	}
}