void draw_pitch(){
	const uint32_t led_to_sample_ratio = (AUTO_CORR_LENGTH>>1) / NUM_LEDS;

	float max_auto_corr = 0.000005;
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		uint32_t sample_index = i * led_to_sample_ratio;
		max_auto_corr = fmaxf(max_auto_corr, auto_corr[sample_index]);
	}

	float auto_scale = 1.0 / max_auto_corr;

	float auto_corr_image[NUM_LEDS];

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = num_leds_float_lookup[i];
		uint32_t sample_index = progress * (AUTO_CORR_LENGTH>>1);
		float auto_corr_level = auto_corr[sample_index] * auto_scale;
		auto_corr_image[i] = clip_float(auto_corr_level);
	}

	dsps_mul_f32(auto_corr_image, auto_corr_image, auto_corr_image, NUM_LEDS, 1, 1, 1);
	//dsps_mul_f32(auto_corr_image, auto_corr_image, auto_corr_image, NUM_LEDS, 1, 1, 1);
	//dsps_mul_f32(auto_corr_image, auto_corr_image, auto_corr_image, NUM_LEDS, 1, 1, 1);
	//dsps_mul_f32(auto_corr_image, auto_corr_image, auto_corr_image, NUM_LEDS, 1, 1, 1);
	//dsps_mul_f32(auto_corr_image, auto_corr_image, auto_corr_image, NUM_LEDS, 1, 1, 1);

	float min_level = 1.0;
	float max_level = 0.0;
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float auto_corr_level = auto_corr_image[i];
		min_level = fminf(min_level, auto_corr_level);
		max_level = fmaxf(max_level, auto_corr_level);
	}

	if(min_level > 0.5){
		//min_level = 1.0;
	}

	float pitch_salience = fabsf(max_level - min_level);
	pitch_salience *= pitch_salience;
	pitch_salience *= pitch_salience;

	if(pitch_salience < 0.45){
		//pitch_salience = 0.00;
	}

	//ESP_LOGI(TAG, "min_level: %.3f, max_level: %.3f, pitch_salience: %.3f", min_level, max_level, pitch_salience);

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = num_leds_float_lookup[i];
		CRGBF pixel_color = hsv(
			get_color_range_hue(progress),
			configuration.saturation.value.f32,
			auto_corr_image[NUM_LEDS-1-i] * pitch_salience
		);

		leds[i] = pixel_color;
	}

	if(configuration.mirror_mode.value.u32 == true){
		memcpy(leds_temp, leds, sizeof(CRGBF) * NUM_LEDS);
		for(uint16_t i = 0; i < (NUM_LEDS>>1); i++){
			leds[(NUM_LEDS>>1) + i    ] = leds_temp[i*2];
			leds[(NUM_LEDS>>1) - 1 - i] = leds_temp[i*2];
		}
	}
}