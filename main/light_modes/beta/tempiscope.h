void draw_tempiscope(){
	start_profile(__COUNTER__, __func__);

	for(uint16_t i = 0; i < NUM_TEMPI; i++){
		float mag = clip_float(tempi_smooth[i]);
		float phase = 1.0 - ((tempi[i].phase + PI) / (2.0*PI));
		
		leds[i] = hsv(
			get_color_range_hue(num_tempi_float_lookup[i]),
			configuration.saturation.value.f32,
			mag*mag
		);
	}

	end_profile();
}
