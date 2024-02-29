void draw_octave() {
	if(configuration.mirror_mode == true){ // Mirror mode
		for (uint16_t i = 0; i < (NUM_LEDS >> 1); i++) {
			float progress = float(i) / (NUM_LEDS >> 1);
			float mag = clip_float(interpolate(progress, chromagram, 12));
			CRGBF color = hsv(configuration.hue+(progress*configuration.hue_range), configuration.saturation, mag);

			leds[63-i] = color;
			leds[64+i] = color;
		}
	}
	else{ // Non mirror
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = float(i) / NUM_LEDS;
			float mag = clip_float(interpolate(progress, chromagram, 12));
			CRGBF color = hsv(configuration.hue+(progress*configuration.hue_range), configuration.saturation, mag);

			leds[i] = color;
		}
	}
}