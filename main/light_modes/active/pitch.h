void draw_pitch(){
	start_profile(__COUNTER__, __func__);
	
	static const uint16_t PITCH_AVERAGE_SAMPLES = 8;
	static const uint32_t led_to_sample_ratio = (AUTO_CORR_LENGTH>>1) / NUM_LEDS;
	float auto_corr_image_average[PITCH_AVERAGE_SAMPLES+1][NUM_LEDS];
	uint16_t average_index = 0;

	// AUTO-SCALE
	float max_auto_corr = 0.004;
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		uint32_t sample_index = i * led_to_sample_ratio;
		max_auto_corr = fmaxf(max_auto_corr, auto_corr[sample_index]);
	}
	float auto_scale = 1.0 / max_auto_corr;

	// SAMPLE AUTO-CORRELATION OUTPUT
	float auto_corr_image[NUM_LEDS];
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = num_leds_float_lookup[i];
		uint32_t sample_index = progress * (AUTO_CORR_LENGTH>>1);
		float auto_corr_level = auto_corr[sample_index] * auto_scale;
		auto_corr_image[i] = clip_float(auto_corr_level);
	}

	// AVERAGE
	dsps_memcpy_aes3(auto_corr_image_average[average_index], auto_corr_image, sizeof(float) * NUM_LEDS);
	average_index = (average_index + 1) % PITCH_AVERAGE_SAMPLES;
	dsps_memset_aes3(auto_corr_image_average[PITCH_AVERAGE_SAMPLES], 0, sizeof(float) * NUM_LEDS);
	for(uint16_t i = 0; i < PITCH_AVERAGE_SAMPLES; i++){
		dsps_add_f32(auto_corr_image_average[PITCH_AVERAGE_SAMPLES], auto_corr_image_average[i], auto_corr_image_average[PITCH_AVERAGE_SAMPLES], NUM_LEDS, 1, 1, 1);
	}
	dsps_mulc_f32(auto_corr_image_average[PITCH_AVERAGE_SAMPLES], auto_corr_image_average[PITCH_AVERAGE_SAMPLES], NUM_LEDS, (1.0 / PITCH_AVERAGE_SAMPLES), 1, 1);

	// DRAW
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = num_leds_float_lookup[i];
		CRGBF pixel_color = hsv(
			get_color_range_hue(progress),
			configuration.saturation.value.f32,
			auto_corr_image_average[PITCH_AVERAGE_SAMPLES][i]*2.0
		);

		leds[i] = pixel_color;
	}

	// MIRROR
	if(configuration.mirror_mode.value.u32 == true){
		dsps_memcpy_aes3(leds_temp, leds, sizeof(CRGBF) * NUM_LEDS);
		for(uint16_t i = 0; i < (NUM_LEDS>>1); i++){
			leds[(NUM_LEDS>>1) + i    ] = leds_temp[i*2];
			leds[(NUM_LEDS>>1) - 1 - i] = leds_temp[i*2];
		}
	}

	end_profile();
}