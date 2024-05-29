void draw_neutral() {
	if(configuration.mirror_mode.value.u32 == true){ // Mirror mode
		for (uint16_t i = 0; i < (NUM_LEDS >> 1); i++) {
			float progress = num_leds_float_lookup[i<<1];
			CRGBF color = hsv(
				get_color_range_hue(progress), 
				configuration.saturation.value.f32,
				1.0
			);

			leds[ (NUM_LEDS>>1)    + i] = color;
			leds[((NUM_LEDS>>1)-1) - i] = color;
		}
	}
	else{ // Non mirror
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float progress = num_leds_float_lookup[i];
			CRGBF color = hsv(
				get_color_range_hue(progress), 
				configuration.saturation.value.f32,
				1.0
			);
			
			leds[i] = color;
		}
	}
}