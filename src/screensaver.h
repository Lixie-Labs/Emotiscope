#define SCREENSAVER_WAIT_MS 5000

extern volatile bool device_touch_active;
extern volatile bool app_touch_active;
extern volatile bool slider_touch_active;

float screensaver_mix = 0.0000;
float screensaver_threshold = 1.0;

float sine_positions[4] = { 0.0, 0.0, 0.0, 0.0 };

uint32_t inactive_start = 0;
bool inactive = false;

void run_screensaver(){
	profile_function([&]() {
		if(configuration.screensaver == false){ return; }
		if(light_modes[configuration.current_mode].type != LIGHT_MODE_TYPE_ACTIVE){ return; }
		
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
		}

		if(device_touch_active == true || app_touch_active == true || slider_touch_active == true){
			inactive = false;
		}

		if(inactive == false){
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
				screensaver_colors[i].r = (screensaver_colors[i].r * incandescent_lookup.r)*0.5 + screensaver_colors[i].r*0.5;
				screensaver_colors[i].g = (screensaver_colors[i].g * incandescent_lookup.g)*0.5 + screensaver_colors[i].g*0.5;
				screensaver_colors[i].b = (screensaver_colors[i].b * incandescent_lookup.b)*0.5 + screensaver_colors[i].b*0.5;
			}
			
			for(uint16_t i = 0; i < 4; i++){
				sine_positions[i] += push_val+(0.0001*i);
				draw_dot(leds, SCREENSAVER_1 + i, screensaver_colors[i], sin(sine_positions[i]) * (0.5*screensaver_mix) + 0.5, screensaver_mix*screensaver_mix);
			}
		}
	}, __func__);
}