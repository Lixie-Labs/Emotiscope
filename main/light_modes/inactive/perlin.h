void draw_perlin(){
	start_profile(__COUNTER__, __func__);

	for (int i = 0; i < NUM_LEDS; i++) {
		float progress = num_leds_float_lookup[i];
		CRGBF color = hsv(
			perlin_noise_array[i>>2] * 0.66,
			configuration.saturation.value.f32,
			0.25
		);
		
		leds[i] = color;
	}

	end_profile();
}