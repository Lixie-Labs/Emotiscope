void draw_fft(){
	start_profile(__COUNTER__, __func__);

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = num_leds_float_lookup[i];
		float mag = fft_smooth[0][i];
		CRGBF color = hsv(
			get_color_range_hue(progress),
			configuration.saturation.value.f32,
			sqrtf(mag)
		);

		leds[i] = color;
	}

	end_profile();
}