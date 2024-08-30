#define NUM_VU_LOG_SAMPLES 20
#define NUM_VU_SMOOTH_SAMPLES 4

float vu_log[NUM_VU_LOG_SAMPLES] = { 0 };
uint16_t vu_log_index = 0;

float vu_smooth[NUM_VU_SMOOTH_SAMPLES] = { 0 };
uint16_t vu_smooth_index = 0;

volatile float vu_level_raw = 0.0;
volatile float vu_level = 0.0;
volatile float vu_max = 0.0;
volatile float vu_floor = 0.0;

uint32_t last_vu_log = 0;

void init_vu(){
	for(uint8_t i = 0; i < NUM_VU_LOG_SAMPLES; i++){
		vu_log[i] = 0.0;
	}
	for(uint8_t i = 0; i < NUM_VU_SMOOTH_SAMPLES; i++){
		vu_smooth[i] = 0.0;
	}
}

void run_vu(){
	start_profile(__COUNTER__, __func__);
	// CALCULATE AMPLITUDE ------------------------------------------------
	static float max_amplitude_cap = 0.0000001;
	float* samples = &sample_history[(SAMPLE_HISTORY_LENGTH-1) - CHUNK_SIZE];

	float max_amplitude_now = 0.000001;
	for(uint16_t i = 0; i < CHUNK_SIZE; i+=1){
		float sample = samples[i];
		float sample_abs = fabs(sample);

		max_amplitude_now = fmaxf(max_amplitude_now, sample_abs);
	}
	max_amplitude_now = clip_float(max_amplitude_now);

	// LOG AMPLITUDE FOR NOISE REMOVAL ------------------------------------
	if(t_now_ms < 2000){
		dsps_memset_aes3(vu_log, max_amplitude_now, sizeof(float)*NUM_VU_LOG_SAMPLES);
	}
	else if(t_now_ms - last_vu_log >= 250){
		last_vu_log = t_now_ms;
		vu_log[vu_log_index] = max_amplitude_now;
		vu_log_index = (vu_log_index + 1) % NUM_VU_LOG_SAMPLES;

		float vu_min = 100000000.0;
		for(uint16_t i = 0; i < NUM_VU_LOG_SAMPLES; i++){
			vu_min = fminf(vu_log[i], vu_min);
		}
		vu_floor = vu_min*1.5;

		//vu_floor *= 0.80;
	}

	// SCALE OUTPUT -------------------------------------------------------
	max_amplitude_now = fmaxf(max_amplitude_now - vu_floor, 0.0f);

	if(max_amplitude_now > max_amplitude_cap){
		float distance = max_amplitude_now - max_amplitude_cap;
		max_amplitude_cap += (distance * 0.1);
	}
	else if(max_amplitude_cap > max_amplitude_now){
		float distance = max_amplitude_cap - max_amplitude_now;
		max_amplitude_cap -= (distance * 0.01);
	}
	max_amplitude_cap = clip_float(max_amplitude_cap);

	float auto_scale = 1.0 / fmaxf(max_amplitude_cap, 0.00001f);

	vu_level_raw = clip_float(max_amplitude_now*auto_scale);

	// SMOOTHING ---------------------------------------------------------
	vu_smooth[vu_smooth_index] = vu_level_raw;
	vu_smooth_index = (vu_smooth_index + 1) % NUM_VU_SMOOTH_SAMPLES;

	float vu_sum = 0.0;
	for(uint8_t i = 0; i < NUM_VU_SMOOTH_SAMPLES; i++){
		vu_sum += vu_smooth[i];
	}
	vu_level = vu_sum / NUM_VU_SMOOTH_SAMPLES;

	// MAX VALUE ---------------------------------------------------------
	vu_max = fmaxf(vu_max, vu_level);

	end_profile();
}