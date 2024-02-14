volatile float vu_level_raw = 0.0;
volatile float vu_level = 0.0;

void run_vu(){
	static float max_amplitude_cap = 0.0000001;
	float* samples = &sample_history[(SAMPLE_HISTORY_LENGTH-1) - CHUNK_SIZE];

	float max_amplitude_now = 0.000001;
	for(uint16_t i = 0; i < CHUNK_SIZE; i++){
		float sample = samples[i];
		float sample_abs = fabs(sample);

		max_amplitude_now = max(max_amplitude_now, sample_abs);
	}
	max_amplitude_now = clip_float(max_amplitude_now);

	if(max_amplitude_now > max_amplitude_cap){
		float distance = max_amplitude_now - max_amplitude_cap;
		max_amplitude_cap += (distance * 0.1);
	}
	else if(max_amplitude_cap > max_amplitude_now){
		float distance = max_amplitude_cap - max_amplitude_now;
		max_amplitude_cap -= (distance * 0.001);
	}
	max_amplitude_cap = clip_float(max_amplitude_cap);

	if(max_amplitude_cap < 0.005){
		max_amplitude_cap = 0.005;
	}

	float auto_scale = 1.0 / max(max_amplitude_cap, 0.00001f);

	vu_level_raw = clip_float(max_amplitude_now*auto_scale);

	float mix_speed = 0.25;
	vu_level = vu_level_raw * mix_speed + vu_level*(1.0-mix_speed);
}