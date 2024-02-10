float novelty_image_prev[NUM_LEDS] = { 0.0 };

float vu_level = 0.000001;

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

void run_vu(){
	static float max_amplitude_now = 0.0000001;
	static float max_amplitude_now_smooth = 0.0000001;
	static float max_amplitude_cap = 0.0000001;

	float* samples = &sample_history[(SAMPLE_HISTORY_LENGTH-1) - CHUNK_SIZE];

	max_amplitude_now = 0.000001;
	for(uint16_t i = 0; i < CHUNK_SIZE; i++){
		float sample = samples[i];
		float sample_abs = fabs(sample);

		max_amplitude_now = max(max_amplitude_now, sample_abs);
	}
	max_amplitude_now = clip_float(max_amplitude_now);

	max_amplitude_now_smooth = max_amplitude_now_smooth*0.95 + max_amplitude_now*0.05;

	if(max_amplitude_now_smooth > max_amplitude_cap){
		float distance = max_amplitude_now_smooth - max_amplitude_cap;
		max_amplitude_cap += (distance * 0.25);
	}
	else if(max_amplitude_cap > max_amplitude_now_smooth){
		float distance = max_amplitude_cap - max_amplitude_now_smooth;
		max_amplitude_cap -= (distance * 0.001);
	}
	max_amplitude_cap = clip_float(max_amplitude_cap);

	if(max_amplitude_cap < 0.01){
		max_amplitude_cap = 0.01;
	}

	float auto_scale = 1.0 / max(max_amplitude_cap, 0.00001f);

	vu_level = clip_float(max_amplitude_now_smooth*auto_scale);
}

void draw_bloom() {
	run_vu();

	float novelty_image[NUM_LEDS] = { 0.0 };

	draw_sprite(novelty_image, novelty_image_prev, NUM_LEDS, NUM_LEDS, 0.25, 0.99);

	novelty_image[0] = (vu_level);
	novelty_image[0] = min( 1.0f, novelty_image[0] );

	if(configuration.mirror_mode == true){
		for(uint16_t i = 0; i < NUM_LEDS>>1; i++){
			float progress = float(i) / (NUM_LEDS >> 1);
			float novelty_pixel = clip_float(novelty_image[i]*2.0);
			CRGBF col = hsv(configuration.hue + progress * configuration.hue_range, 1.0, novelty_pixel*novelty_pixel);
			leds[64+i] = col;
			leds[63-i] = col;
		}
	}
	else{
		for(uint16_t i = 0; i < NUM_LEDS; i++){
			float progress = float(i) / (NUM_LEDS);
			float novelty_pixel = clip_float(novelty_image[i]*2.0);
			CRGBF col = hsv(configuration.hue + progress * configuration.hue_range, 1.0, novelty_pixel*novelty_pixel);
			leds[i] = col;
		}
	}

	memcpy(novelty_image_prev, novelty_image, sizeof(float)*NUM_LEDS);
}