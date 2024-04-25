void draw_spectrum() {
	// Mirror mode
	if(configuration.mirror_mode == true){
		for (uint16_t i = 0; i < NUM_LEDS>>1; i++) {
			float progress = float(i) / (NUM_LEDS>>1);
			float mag = (spectrogram_smooth[i]);
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation,
				mag
			);

			leds[63-i] = color;
			leds[64+i] = color;
		}
	}
	// Non mirror
	else{ 
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = float(i) / NUM_LEDS;
			float mag = (clip_float(interpolate(progress, spectrogram_smooth, NUM_FREQS)));
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation,
				mag
			);

			leds[i] = color;
		}
	}
}