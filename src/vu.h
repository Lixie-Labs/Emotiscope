#define NUM_VU_AVERAGE_SAMPLES 4

extern uint32_t noise_calibration_active_frames_remaining;

volatile float vu_level_raw = 0.0;
volatile float vu_level = 0.0;
volatile float vu_max = 0.0;

float vu_history[NUM_VU_AVERAGE_SAMPLES] = { 0 };
uint8_t vu_history_index = 0;

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

	if(noise_calibration_active_frames_remaining == 0){ // Not calibrating
		max_amplitude_now = clip_float(max_amplitude_now - configuration.vu_floor);
	}
	else{ // Calibrating
		configuration.vu_floor = max(configuration.vu_floor, max_amplitude_now/2.0);
	}

	if(max_amplitude_now > max_amplitude_cap){
		float distance = max_amplitude_now - max_amplitude_cap;
		max_amplitude_cap += (distance * 0.1);
	}
	else if(max_amplitude_cap > max_amplitude_now){
		float distance = max_amplitude_cap - max_amplitude_now;
		max_amplitude_cap -= (distance * 0.01);
	}
	max_amplitude_cap = clip_float(max_amplitude_cap);

	if(max_amplitude_cap < 0.0005){
		max_amplitude_cap = 0.0005;
	}

	float auto_scale = 1.0 / max(max_amplitude_cap, 0.00001f);

	vu_level_raw = clip_float(max_amplitude_now*auto_scale);

	float mix_speed = 0.25;
	vu_level = vu_level_raw * mix_speed + vu_level*(1.0-mix_speed);

	vu_history[vu_history_index] = vu_level;
	vu_history_index = (vu_history_index + 1) % NUM_VU_AVERAGE_SAMPLES;

	float vu_sum = 0.0;
	for(uint8_t i = 0; i < NUM_VU_AVERAGE_SAMPLES; i++){
		vu_sum += vu_history[i];
	}
	vu_level = vu_sum / NUM_VU_AVERAGE_SAMPLES;

	vu_max = max(vu_max, vu_level);
}