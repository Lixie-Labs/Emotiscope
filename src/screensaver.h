#define SCREENSAVER_WAIT_MS 2500

float screensaver_mix = 0.0000;
float screensaver_threshold = 1.0;

float sine_positions[4] = { 0.0, 0.0, 0.0, 0.0 };

uint32_t inactive_start = 0;
bool inactive = false;

void run_screensaver(uint32_t t_now_ms){
	float mag_sum = 0;
	for(uint16_t i = 0; i < NUM_FREQS; i++){
		mag_sum += frequencies_musical[i].magnitude;
	}

	if(mag_sum < screensaver_threshold){
		if(inactive == false){
			inactive = true;
			inactive_start = t_now_ms;
		}
	}
	else if(mag_sum > screensaver_threshold){
		inactive = false;

		if(screensaver_mix > 0.0){
			screensaver_mix -= 0.01;
		}
	}

	if(inactive == true){
		if(t_now_ms - inactive_start >= SCREENSAVER_WAIT_MS){
			if(screensaver_mix < 1.0){
				screensaver_mix += 0.001;
			}
		}
	}

	if(screensaver_mix > 0.001){
		scale_CRGBF_array_by_constant(leds, 1.0-(screensaver_mix*screensaver_mix), NUM_LEDS);

		const float push_val = 0.005;
		CRGBF screensaver_colors[4] = {
			{1.0,  0.0,  0.0},
			{1.0,  0.25, 0.0},
			{0.0,  1.0,  0.0},
			{0.0,  0.25, 1.0},
		};

		for(uint16_t i = 0; i < 4; i++){
			screensaver_colors[i].r *= incandescent_lookup.r;
			screensaver_colors[i].g *= incandescent_lookup.g;
			screensaver_colors[i].b *= incandescent_lookup.b;
		}
		
		for(uint16_t i = 0; i < 4; i++){
			sine_positions[i] += push_val+(0.0001*i);
			draw_dot(leds, SCREENSAVER_1 + i, screensaver_colors[i], sin(sine_positions[i]) * (0.5*screensaver_mix) + 0.5, screensaver_mix*screensaver_mix);
		}
	}
}