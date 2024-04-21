void draw_debug(){
	for(uint16_t i = 0; i < 128; i++){
		int32_t index = ((NOVELTY_HISTORY_LENGTH-1)-128)+i;
		float mag_vu = vu_curve[index];
		float mag_spec = novelty_curve_normalized[index];

		CRGBF dot_color = {
			mag_vu*mag_vu,
			mag_spec*mag_spec,
			0.0,
		};

		leds[i] = dot_color;
	}
}