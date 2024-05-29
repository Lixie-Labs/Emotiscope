float samples[NUM_LEDS];

void draw_waveform(){
	memcpy(samples, &sample_history[(SAMPLE_HISTORY_LENGTH-1) - (NUM_LEDS+CHUNK_SIZE)], sizeof(float) * NUM_LEDS);
	float cutoff_frequency = 110 + 2000*(1.0/*-configuration.bass*/);
	low_pass_filter(samples, NUM_LEDS, SAMPLE_RATE, cutoff_frequency, 3);

	float max_val = 0.000001;
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float sample = clip_float(samples[i]);

		max_val = max(max_val, sample);
	}

	float auto_scale = 1.0 / max_val;

	if(configuration.mirror_mode.value.u32 == false){
		for(uint16_t i = 0; i < NUM_LEDS; i++){
			float progress = float(i) / NUM_LEDS;
			float sample = clip_float(samples[i]) * auto_scale;
			CRGBF pixel_color = hsv(
				configuration.color.value.f32 + (configuration.color_range.value.f32*progress),
				configuration.saturation.value.f32,
				sample
			);

			leds[i] = pixel_color;
		}
	}
	else{
		for(uint16_t i = 0; i < NUM_LEDS >> 1; i++){
			float progress = float(i) / (NUM_LEDS>>1);
			float sample = clip_float(samples[i]) * auto_scale;
			CRGBF pixel_color = hsv(
				configuration.color.value.f32 + (configuration.color_range.value.f32*progress),
				configuration.saturation.value.f32,
				sample
			);

			leds[63-i] = pixel_color;
			leds[64+i] = pixel_color;
		}
	}
}