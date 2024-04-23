void draw_spectrum() {
	if(configuration.mirror_mode == true){ // Mirror mode
		for (uint16_t i = 0; i < NUM_LEDS>>1; i++) {
			float progress = float(i) / (NUM_LEDS>>1);
			float mag = sqrt(spectrogram_smooth[i]);
			// TODO: Make "base coat" a slider in the web app for (at least) Spectrum Mode
			// mag = mag * 0.99 + 0.01;
			CRGBF color = hsv(configuration.color+(progress*configuration.color_range), configuration.saturation, mag);

			// TODO: Make "saturation" a slider in the web app
			
			leds[63-i] = color;
			leds[64+i] = color;
		}
	}
	else{ // Non mirror
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = float(i) / NUM_LEDS;
			float mag = sqrt(clip_float(interpolate(progress, spectrogram_smooth, NUM_FREQS)));
			CRGBF color = hsv(configuration.color+(progress*configuration.color_range), configuration.saturation, mag);

			leds[i] = color;
		}
	}
}