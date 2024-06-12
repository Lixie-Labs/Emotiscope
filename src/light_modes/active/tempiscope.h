void draw_tempiscope(){
	// Draw the current frame
	for(uint16_t i = 0; i < NUM_TEMPI; i++){
		float progress = num_leds_float_lookup[i];

		float sine = 1.0 - ((tempi[i].phase + PI) / (2.0*PI));

		float mag = clip_float(tempi_smooth[i] * sine);

		if(mag > 0.005){
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				mag
			);

			leds[i] = color;
		}
	}
}