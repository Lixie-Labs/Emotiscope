void draw_debug(){
	for(uint16_t i = 0; i < NUM_TEMPI; i++){
		float tempo_magnitude = tempi_smooth[i];
		float contribution = clip_float(tempo_magnitude / tempi_power_sum);
		
		float phase_linear = (tempi[i].phase + PI) / (PI*2.0);
		if(tempi[i].phase_inverted == true){
			phase_linear = 1.0 - phase_linear;
		}

		//phase_linear *= 0.5;

		CRGBF dot_color = hsv(i / (float)NUM_TEMPI, 1.0, 1.0);
		draw_dot(leds, NUM_RESERVED_DOTS + (i*2+0), dot_color, phase_linear, contribution);
		//draw_dot(leds, NUM_RESERVED_DOTS + (i*2+1), dot_color, 1.0-phase_linear, contribution*contribution);
	}
}

void draw_debug_old(){
	for(uint16_t i = 0; i < NUM_TEMPI; i++){
		float tempo_magnitude = tempi_smooth[i];
		float contribution = clip_float(tempo_magnitude / tempi_power_sum);
		
		float phase_linear = ((tempi[i].phase*-1.0) + (PI*2.0)) / (PI*4.0);
		if(tempi[i].phase_inverted == true){
			phase_linear = 1.0 - phase_linear;
		}

		// Draw the tempi
		float brightness = phase_linear*phase_linear;

		leds[i] = hsv(i / (float)NUM_TEMPI, 1.0, brightness*contribution);
	}
}