void draw_debug(){
	start_profile(__COUNTER__, __func__);

	int16_t scale_ratio = NOVELTY_HISTORY_LENGTH / NUM_LEDS;

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		// Get the average of the novelty curve for this LED
		float sum = 0.0;
		for(uint16_t j = 0; j < scale_ratio; j++){
			sum += novelty_curve[(i*scale_ratio)+j];
		}
		float novelty = sum / scale_ratio;

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