void draw_fft(){
	start_profile(__COUNTER__, __func__);

	if(configuration.mirror_mode.value.u32 == false){
		for(uint16_t i = 0; i < NUM_LEDS; i++){
			float progress = num_leds_float_lookup[i];
			float mag = fft_smooth[0][i];

			if(i >= 16 && i < 96){
				//mag = 0;
			}

			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				(mag)
			);

			leds[i] = color;
		}
	}
	
	else if(configuration.mirror_mode.value.u32 == true){
		for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
			float progress = num_leds_float_lookup[i<<1];
			float mag = ( fft_smooth[0][(i<<1) + 0] + fft_smooth[0][(i<<1) + 1] ) * 0.5f;
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				(mag)
			);

			leds[(NUM_LEDS>>1)+i]         = color;
			leds[((NUM_LEDS>>1) - 1) - i] = color;
		}
	}
	

	end_profile();
}