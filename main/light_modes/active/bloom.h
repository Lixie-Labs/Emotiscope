float novelty_image_prev[NUM_LEDS] = { 0.0 };

void draw_bloom() {
	float novelty_image[NUM_LEDS] = { 0.0 };

	float spread_speed = 0.125 + 0.875*configuration.speed.value.f32;
	draw_sprite(novelty_image, novelty_image_prev, NUM_LEDS, NUM_LEDS, spread_speed, 0.99);

	novelty_image[0] = (vu_level);
	novelty_image[0] = min( 1.0f, novelty_image[0] );

	if(configuration.mirror_mode.value.u32 == true){
		for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
			float progress = num_leds_float_lookup[i<<1];
			float novelty_pixel = clip_float(novelty_image[i]*1.0);
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				novelty_pixel
			);
			leds[ (NUM_LEDS>>1)    + i] = color;
			leds[((NUM_LEDS>>1)-1) - i] = color;
		}
	}
	else{
		for(uint16_t i = 0; i < NUM_LEDS; i++){
			float progress = num_leds_float_lookup[i];
			float novelty_pixel = clip_float(novelty_image[i]*2.0);
			CRGBF color = hsv(
				get_color_range_hue(progress),
				configuration.saturation.value.f32,
				novelty_pixel
			);
			leds[i] = color;
		}
	}

	memcpy(novelty_image_prev, novelty_image, sizeof(float)*NUM_LEDS);
}