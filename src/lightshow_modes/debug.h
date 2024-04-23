void draw_debug_novelty(){
	for(uint16_t i = 0; i < 128; i++){
		int32_t index = ((NOVELTY_HISTORY_LENGTH-1)-128)+i;
		float mag_vu = vu_curve[index];
		float mag_spec = novelty_curve_normalized[index];

		CRGBF dot_color = {
			mag_vu,
			mag_spec,
			0.0,
		};

		leds[i] = dot_color;
	}
}

void draw_debug(){
	for(uint16_t i = 0; i < 64; i++){
		float progress = float(i) / 64;
		leds[i] = hsv(configuration.color + progress*configuration.color_range, 1.0, 1.0);
	}
	for(uint16_t i = 0; i < 64; i++){
		float progress = float(i) / 64;
		leds[64+i] = hsv(configuration.color + configuration.color_range, 1.0, 1.0 - progress);
	}
}