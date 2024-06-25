void draw_fft(){
	static float auto_scale_smooth = 0.001;

	uint16_t size_diff = (FFT_SIZE>>1) / NUM_LEDS;

	float fft_mags[NUM_LEDS] = { 0.0 };
	memset(fft_mags, 0, sizeof(float) * NUM_LEDS);

	float fft_max_mag = 0.0;
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		if(i >= 4){
			fft_mags[i] = fft_smooth[0][i];
			fft_max_mag = fmaxf(fft_max_mag, fft_mags[i]);
		}
	}

	float auto_scale = 1.0 / fmaxf(fft_max_mag, 0.0001f);
	auto_scale_smooth = auto_scale_smooth * 0.99 + auto_scale * 0.01;

	dsps_mulc_f32(fft_mags, fft_mags, NUM_LEDS, auto_scale_smooth, 1, 1);

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = num_leds_float_lookup[i];
		float mag = clip_float(fft_mags[i]);
		CRGBF color = hsv(
			get_color_range_hue(progress),
			configuration.saturation.value.f32,
			mag
		);

		leds[i] = color;
	}
}