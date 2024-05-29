float random_float(float min, float max) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}

void draw_starfield(){
	for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
		float progress = (float)i / (float)(NUM_LEDS>>1);
		leds[i] = hsv(
			1.0,
			configuration.saturation.value.f32,
			progress*1.0
		);
	}

	for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
		float progress = (float)i / (float)(NUM_LEDS>>1);
		leds[(NUM_LEDS >> 1) + i] = hsv(
			1.0,
			configuration.saturation.value.f32,
			1.0 + (progress*3.0)
		);
	}
}

void draw_starfield_real(){
	const uint16_t num_stars = 150;
	static float star_speed[num_stars];
	static float star_positions[num_stars];
	static float star_brightness[num_stars];

	static bool first_run = true;
	if(first_run){
		memset(star_speed,     0, sizeof(float)*num_stars);
		memset(star_positions, 0, sizeof(float)*num_stars);
		memset(star_brightness,0, sizeof(float)*num_stars);
		first_run = false;
	}

	for(uint16_t i = 0; i < num_stars; i++){
		float progress = (float)i / (float)num_stars;
		CRGBF dot_color = hsv(
				get_color_range_hue(progress), 
				configuration.saturation.value.f32,
				star_brightness[i]*4.0
			);

		if(fabsf(star_speed[i]) <= 0.01 || star_brightness[i] < 0.001){
			star_speed[i] = (random_float(0.0, 1.0) - 0.5) * 0.1;
			star_brightness[i] = 1.0; //random_float(0.1, 1.0);
			star_positions[i] = 0.0;
			
			dot_color = {
				0.00001,
				0.00001,
				0.00001
			};
		}
		else{
			star_positions[i] += (star_speed[i]*0.1);
			star_brightness[i] *= 0.975;

			if(fabsf( star_positions[i] ) > 1.25f){
				star_speed[i] = 0.0;
			}
		}

		draw_dot(leds, NUM_RESERVED_DOTS+i, dot_color, 0.5 + (star_positions[i] * 0.5), 1.0);
	}
}