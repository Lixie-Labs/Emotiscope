float max_amplitude_now = 0.0000001;
float max_amplitude_now_smooth = 0.0000001;
float max_amplitude_cap = 0.0000001;

void draw_vu(){
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

	if(max_amplitude_cap < 0.005){
		max_amplitude_cap = 0.005;
	}

	float auto_scale = 1.0 / max(max_amplitude_cap, 0.00001f);

	float dot_pos = clip_float(max_amplitude_now_smooth*auto_scale);
	CRGBF dot_color = hsv(configuration.hue + configuration.hue_range*dot_pos, 1.0, 1.0);

	if(configuration.mirror_mode == true){
		draw_dot(leds, NUM_RESERVED_DOTS+0, dot_color, 0.5 + (dot_pos* 0.5), 1.0);
		draw_dot(leds, NUM_RESERVED_DOTS+1, dot_color, 0.5 + (dot_pos*-0.5), 1.0);
	}
	else{
		draw_dot(leds, NUM_RESERVED_DOTS+0, dot_color, dot_pos, 1.0);
	}
}