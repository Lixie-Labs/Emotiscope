float novelty_image_prev[NUM_LEDS] = { 0.0 };

void draw_bloom() {
	float novelty_image[NUM_LEDS] = { 0.0 };

	float spread_speed = 0.125 + 0.875*configuration.speed;
	draw_sprite(novelty_image, novelty_image_prev, NUM_LEDS, NUM_LEDS, spread_speed, 0.99);

	novelty_image[0] = (vu_level);
	novelty_image[0] = min( 1.0f, novelty_image[0] );

	if(configuration.mirror_mode == true){
		for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
			float progress = float(i) / (NUM_LEDS >> 1);
			float novelty_pixel = clip_float(novelty_image[i]*1.0);
			CRGBF col = hsv(configuration.color + progress * configuration.color_range, configuration.saturation, novelty_pixel*novelty_pixel);
			leds[64+i] = col;
			leds[63-i] = col;
		}
	}
	else{
		for(uint16_t i = 0; i < NUM_LEDS; i++){
			float progress = float(i) / (NUM_LEDS);
			float novelty_pixel = clip_float(novelty_image[i]*2.0);
			CRGBF col = hsv(configuration.color + progress * configuration.color_range, configuration.saturation, novelty_pixel*novelty_pixel);
			leds[i] = col;
		}
	}

	memcpy(novelty_image_prev, novelty_image, sizeof(float)*NUM_LEDS);
}