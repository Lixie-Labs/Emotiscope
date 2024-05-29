void draw_hype() {
	float beat_sum_odd  = 0.0;
	float beat_sum_even = 0.0;

	// Draw tempi to the display
	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float tempi_magnitude = tempi_smooth[tempo_bin];
		float contribution = ((tempi_magnitude * tempi_magnitude) / tempi_power_sum) * tempi_magnitude;

		contribution *= tempi[tempo_bin].beat * 0.5 + 0.5;

		if(tempo_bin % 2 == 0){
			beat_sum_even += contribution;
		} else {
			beat_sum_odd += contribution;
		}
	}
	beat_sum_odd  = clip_float(beat_sum_odd);
	beat_sum_even = clip_float(beat_sum_even);

	float beat_color_odd  = beat_sum_odd;
	float beat_color_even = beat_sum_even;
	beat_sum_odd  = sqrt(sqrt(beat_color_odd));
	beat_sum_even = sqrt(sqrt(beat_color_even));

	float strength = sqrt(tempo_confidence);

	CRGBF dot_color_odd  = hsv(
		get_color_range_hue(beat_color_odd),
		configuration.saturation.value.f32,
		1.0
	);
	CRGBF dot_color_even = hsv(
		get_color_range_hue(beat_color_even+0.5*configuration.color_range.value.f32),
		configuration.saturation.value.f32,
		1.0
	);

	if(configuration.mirror_mode.value.u32 == true){
		beat_sum_odd  *= 0.5;
		beat_sum_even *= 0.5;
	}

	draw_dot(leds, NUM_RESERVED_DOTS + 0, dot_color_odd,  1.0-beat_sum_odd,  0.1 + 0.8*strength);
	draw_dot(leds, NUM_RESERVED_DOTS + 1, dot_color_even, 1.0-beat_sum_even, 0.1 + 0.8*strength);

	if(configuration.mirror_mode.value.u32 == true){
		draw_dot(leds, NUM_RESERVED_DOTS + 2, dot_color_odd,  beat_sum_odd,  0.1 + 0.8*strength);
		draw_dot(leds, NUM_RESERVED_DOTS + 3, dot_color_even, beat_sum_even, 0.1 + 0.8*strength);
	}
}