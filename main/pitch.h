#define AUTO_CORR_LENGTH 256

float auto_corr[AUTO_CORR_LENGTH];

float downsampled_history_filtered[SAMPLE_HISTORY_LENGTH];

const float low_frequency = 600.0;
const float low_q_factor = 1.0;
float low_coeffs_lpf[5];
float low_w_lpf[5] = {0, 0};

const float high_frequency = 1500.0;
const float high_q_factor = 1.0;
float high_coeffs_lpf[5];
float high_w_lpf[5] = {0, 0};

void init_low_filter(){
	esp_err_t ret = ESP_OK;
    
    ret = dsps_biquad_gen_lpf_f32(low_coeffs_lpf, (low_frequency / (SAMPLE_RATE>>1)), low_q_factor);
    if (ret  != ESP_OK) {
        ESP_LOGE(TAG, "Operation error = %i", ret);
        return;
    }
}

void init_high_filter(){
	esp_err_t ret = ESP_OK;
    
    ret = dsps_biquad_gen_lpf_f32(high_coeffs_lpf, (high_frequency / (SAMPLE_RATE>>1)), high_q_factor);
    if (ret  != ESP_OK) {
        ESP_LOGE(TAG, "Operation error = %i", ret);
        return;
    }
}

void init_pitch_filters(){
	init_low_filter();
	init_high_filter();
}

void apply_pitch_filters(){
	// HPF
	dsps_biquad_f32(downsampled_history, downsampled_history_filtered, SAMPLE_HISTORY_LENGTH, low_coeffs_lpf, low_w_lpf);
	for(uint16_t i = 0; i < SAMPLE_HISTORY_LENGTH; i+=4){ // LPF to HPF
		downsampled_history_filtered[i+0] = downsampled_history[i+0] - downsampled_history_filtered[i+0];
		downsampled_history_filtered[i+1] = downsampled_history[i+1] - downsampled_history_filtered[i+1];
		downsampled_history_filtered[i+2] = downsampled_history[i+2] - downsampled_history_filtered[i+2];
		downsampled_history_filtered[i+3] = downsampled_history[i+3] - downsampled_history_filtered[i+3];
	}

	// LPF
	dsps_biquad_f32(downsampled_history_filtered, downsampled_history_filtered, SAMPLE_HISTORY_LENGTH, high_coeffs_lpf, high_w_lpf);

}

void estimate_pitch(){
	apply_pitch_filters();

	float* sample_ptr = &downsampled_history_filtered[(SAMPLE_HISTORY_LENGTH - 1) - AUTO_CORR_LENGTH];

	float windowed_pattern[AUTO_CORR_LENGTH>>1];
	dsps_memcpy_aes3(windowed_pattern, sample_ptr, sizeof(float) * (AUTO_CORR_LENGTH>>1));

	const static uint16_t window_scale_ratio = 4096 / (AUTO_CORR_LENGTH>>1);
	for(uint16_t i = 0; i < AUTO_CORR_LENGTH>>1; i+=4){
		windowed_pattern[i+0] *= window_lookup[(i+0)*window_scale_ratio];
		windowed_pattern[i+1] *= window_lookup[(i+1)*window_scale_ratio];
		windowed_pattern[i+2] *= window_lookup[(i+2)*window_scale_ratio];
		windowed_pattern[i+3] *= window_lookup[(i+3)*window_scale_ratio];
	}

	dsps_corr_f32(
		sample_ptr,
		AUTO_CORR_LENGTH,
		windowed_pattern,
		AUTO_CORR_LENGTH>>1,
		auto_corr
	);
	auto_corr[0] = auto_corr[1];

	for(uint16_t i = 0; i < 8; i++){
		float progress = i / 8.0;
		auto_corr[i] *= progress;
	}
}