void draw_hype() {
	float beat_sum = 0.0;

	// Draw tempi to the display
	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float tempi_magnitude = tempi_smooth[tempo_bin];
		float contribution = ((tempi_magnitude * tempi_magnitude) / tempi_power_sum) * tempi_magnitude;

		contribution *= tempi[tempo_bin].beat * 0.5 + 0.5;

		beat_sum += contribution;
	}
	beat_sum = clip_float(beat_sum);

	float beat_hue = beat_sum;
	beat_sum = sqrt(beat_hue);

	CRGBF dot_color = hsv(configuration.hue + beat_hue*configuration.hue_range, 1.0, 1.0);

	if(configuration.mirror_mode == true){
		beat_sum *= 0.5;
	}

	draw_dot(leds, 0, dot_color, 1.0-beat_sum, 1.0);

	if(configuration.mirror_mode == true){
		draw_dot(leds, 1, dot_color, beat_sum, 1.0);
	}
}