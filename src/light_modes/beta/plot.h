void draw_line(float* layer, float x1, float x2, float opacity) {
	if (x1 > x2) {	// Ensure x1 <= x2
		float temp = x1;
		x1 = x2;
		x2 = temp;
	}

	float ix1 = floor(x1);
	float ix2 = ceil(x2);

	// start pixel
	if (ix1 >= 0 && ix1 < NUM_LEDS) {
		float coverage = 1.0 - (x1 - ix1);
		float mix = opacity * coverage;

		layer[uint16_t(ix1)] += mix;
	}

	// end pixel
	if (ix2 >= 0 && ix2 < 128) {
		float coverage = x2 - floor(x2);
		float mix = opacity * coverage;

		layer[uint16_t(ix2)] += mix;
	}

	// pixels in between
	for (float i = ix1 + 1; i < ix2; i++) {
		if (i >= 0 && i < NUM_LEDS) {
			layer[uint16_t(i)] += opacity;
		}
	}
}

void draw_plot(){
	static float image[NUM_LEDS];

	//if(waveform_locked == false && waveform_sync_flag == true){
		//waveform_sync_flag = false;
		
		memset(image, 0, sizeof(float)*NUM_LEDS);

		const uint16_t num_samples = 128;
		float* samples_raw = &sample_history[(SAMPLE_HISTORY_LENGTH-1) - (num_samples)];
		float samples[num_samples];
		memcpy(samples, samples_raw, sizeof(float)*num_samples);

		float max_stretch = 0.025;
		for(uint16_t i = 0; i < num_samples; i+=1){
			float sample = samples[i];
			max_stretch = max(max_stretch, fabs(sample));
		}
		float auto_stretch = 1.0 / max_stretch;

		for(uint16_t i = 0; i < num_samples; i+=1){
			samples[i] = samples[i]*auto_stretch;
			samples[i] = samples[i]*62.0 + 64;
		}

		const uint16_t num_iterations = 1000;
		const float step_size = 1.0 / num_iterations;
		float progress = 0.0;
		for(uint16_t i = 0; i < num_iterations; i++){
			progress += step_size;
			
			float sample = interpolate(progress, samples, (num_samples-1));
			uint16_t sample_whole = sample;
			float sample_fract = sample - sample_whole;

			image[sample_whole  ] += (1.0-sample_fract);
			image[sample_whole+1] += (sample_fract);
		}
	//}

	float max_val = 0.0000001;	
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		max_val = max(max_val, image[i]);
	}

	float auto_scale = 1.0 / (max_val);

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = float(i) / NUM_LEDS;
		float pixel = clip_float( image[i] * auto_scale );
		pixel *= pixel;
		CRGBF pixel_color = hsv(configuration.color.value.f32 + linear_to_tri(progress)*configuration.color_range.value.f32, configuration.saturation.value.f32, pixel);
		leds[i] = pixel_color;
	}
}