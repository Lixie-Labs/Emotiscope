void draw_neutral() {
	if(configuration.mirror_mode == true){ // Mirror mode
		for (uint16_t i = 0; i < (NUM_LEDS >> 1); i++) {
			float progress = float(i) / (NUM_LEDS >> 1);
			CRGBF color = hsv(
				get_color_range_hue(progress), 
				configuration.saturation,
				1.0
			);

			leds[63-i] = color;
			leds[64+i] = color;
		}
	}
	else{ // Non mirror
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = float(i) / NUM_LEDS;
			CRGBF color = hsv(
				get_color_range_hue(progress), 
				configuration.saturation,
				1.0
			);
			
			leds[i] = color;
		}
	}
}