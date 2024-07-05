void draw_debug(){
	start_profile(__COUNTER__, __func__);

	for(uint16_t i = 0; i < 128; i++){
		int32_t index = ((NOVELTY_HISTORY_LENGTH-1)-128)+i;
		float mag_spec = novelty_curve_normalized[index];

		CRGBF dot_color = {
			0.0,
			mag_spec*mag_spec,
			0.0,
		};

		leds[i] = dot_color;
	}

	float dot_pos = silence_level;
	draw_dot(leds, NUM_RESERVED_DOTS+0, (CRGBF){0.25, 0.0, 0.0}, dot_pos, 1.0);

	end_profile();
}