float screensaver_mix = 0.0000;
float screensaver_threshold = 1.0;

float sine_positions[4] = { 0.0, 0.0, 0.0, 0.0 };

void run_screensaver(){
	float mag_sum = 0;
	for(uint16_t i = 0; i < NUM_FREQS; i++){
		mag_sum += frequencies_musical[i].magnitude;
	}

	if(mag_sum < screensaver_threshold){
		if(screensaver_mix < 1.0){
			screensaver_mix += 0.001;
		}
	}
	else if(mag_sum > screensaver_threshold){
		if(screensaver_mix > 0.0){
			screensaver_mix -= 0.01;
		}
		else{
			//memset(sine_positions, 0, sizeof(float));
		}
	}

	if(screensaver_mix > 0.001){
		const float push_val = 0.005;
		CRGBF screensaver_colors[4] = {
			{1.0,  0.0,  0.0},
			{1.0,  0.25, 0.0},
			{0.0,  1.0,  0.0},
			{0.0,  0.25, 1.0},
		};

		
		for(uint16_t i = 0; i < 4; i++){
			sine_positions[i] += push_val+(0.0001*i);
			draw_dot(leds, SCREENSAVER_1 + i, screensaver_colors[i], sin(sine_positions[i]) * (0.5*screensaver_mix) + 0.5, screensaver_mix*screensaver_mix);
		}
	}
}