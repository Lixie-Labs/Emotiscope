void draw_sprite(float dest[], float sprite[], uint32_t dest_length, uint32_t sprite_length, float position, float alpha) {
  int32_t position_whole = position;  // Downcast to integer accuracy
  float position_fract = position - position_whole;
  float mix_right = position_fract;
  float mix_left = 1.0 - mix_right;

  for (uint16_t i = 0; i < sprite_length; i++) {
    int32_t pos_left = i + position_whole;
    int32_t pos_right = i + position_whole + 1;

    bool skip_left = false;
    bool skip_right = false;

    if (pos_left < 0) {
      pos_left = 0;
      skip_left = true;
    }
    if (pos_left > dest_length - 1) {
      pos_left = dest_length - 1;
      skip_left = true;
    }

    if (pos_right < 0) {
      pos_right = 0;
      skip_right = true;
    }
    if (pos_right > dest_length - 1) {
      pos_right = dest_length - 1;
      skip_right = true;
    }

    if (skip_left == false) {
      dest[pos_left] += sprite[i] * mix_left * alpha;
    }

    if (skip_right == false) {
      dest[pos_right] += sprite[i] * mix_right * alpha;
    }
  }
}

float novelty_image_prev[NUM_LEDS] = { 0.0 };

void draw_bloom() {
	float novelty_image[NUM_LEDS] = { 0.0 };

	draw_sprite(novelty_image, novelty_image_prev, NUM_LEDS, NUM_LEDS, 0.25, 0.995);

	novelty_image[0] += novelty_curve_normalized[NOVELTY_HISTORY_LENGTH - 1];
	novelty_image[0] = min( 1.0f, novelty_image[0] );

	for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
		float novelty_pixel = novelty_image[i];

		CRGBF col = hsv(0.0, 1.0, novelty_pixel*novelty_pixel);
		leds[64+i] = col;
		leds[63-i] = col;
	}

	memcpy(novelty_image_prev, novelty_image, sizeof(float)*NUM_LEDS);
}