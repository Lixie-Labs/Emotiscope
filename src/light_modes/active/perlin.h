void draw_perlin(){
	static float perlin_image_hue[NUM_LEDS];
	static float perlin_image_lum[NUM_LEDS];

	static double x = 0.00;
	static double y = 0.00;

	static float momentum = 0.0;

	float push = vu_level*vu_level*vu_level*vu_level*configuration.speed*0.1f;

	momentum *= 0.99;

	momentum = max(momentum, push);

	static float angle = 0.0;
	angle += 0.001;
	float sine = sin(angle);

	x += 0.01*sine;

	y += 0.0001;
	y += momentum;

	fill_array_with_perlin(perlin_image_hue, NUM_LEDS, (float)x, (float)y, 0.025f);
	fill_array_with_perlin(perlin_image_lum, NUM_LEDS,  (float)x+100, (float)y+50, 0.0125f);

	// Crazy SIMD functions scaling perlin_image_lum from 0.0 - 1.0 range to 0.1 - 1.0 range
	float* ptr = (float*)perlin_image_lum;
	dsps_mulc_f32_ae32(ptr, ptr, NUM_LEDS, 0.98, 1, 1);
	dsps_addc_f32_ae32(ptr, ptr, NUM_LEDS, 0.02, 1, 1);
	
	if(configuration.mirror_mode == false){
		for(uint16_t i = 0; i < NUM_LEDS; i++){
			CRGBF color = hsv(
				get_color_range_hue(perlin_image_hue[i]),
				configuration.saturation,
				perlin_image_lum[i]*perlin_image_lum[i]
			);

			leds[i] = color;
		}
	}
	else{
		for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
			CRGBF color = hsv(
				get_color_range_hue(perlin_image_hue[i<<1]),
				configuration.saturation,
				perlin_image_lum[i<<1]*perlin_image_lum[i<<1]
			);

			leds[i] = color;
			leds[NUM_LEDS - 1 - i] = color;
		}
	}
}