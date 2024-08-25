void draw_debug(){
	start_profile(__COUNTER__, __func__);

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float novelty = novelty_curve[(NOVELTY_HISTORY_LENGTH-1) - NUM_LEDS + i] * novelty_scale_factor;

		CRGBF dot_color = {
			0.0,
			(novelty * num_leds_float_lookup[i]),
			0.0,
		};

		leds[i] = add(leds[i], dot_color);
	}

	float dot_pos = silence_level;
	//draw_dot(leds, NUM_RESERVED_DOTS+0, (CRGBF){0.25, 0.0, 0.0}, dot_pos, 1.0);

	end_profile();
}