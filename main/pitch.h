#define AUTO_CORR_LENGTH 256

float auto_corr[AUTO_CORR_LENGTH];

float temp_array[AUTO_CORR_LENGTH];

void estimate_pitch(){
	float* sample_ptr = &downsampled_history[(SAMPLE_HISTORY_LENGTH - 1) - AUTO_CORR_LENGTH];
	dsps_corr_f32_ansi(
		sample_ptr,
		AUTO_CORR_LENGTH,
		sample_ptr + (AUTO_CORR_LENGTH >> 1),
		AUTO_CORR_LENGTH>>1,
		auto_corr
	);
	auto_corr[0] = auto_corr[1];

	//dsps_mul_f32(auto_corr, auto_corr, auto_corr, AUTO_CORR_LENGTH, 1, 1, 1);

	// Normalize
	dsps_mulc_f32(auto_corr, auto_corr, AUTO_CORR_LENGTH, 1.0, 1, 1);

	static uint16_t iter = 0;
	if(iter == 100){
		ESP_LOGI(TAG, "Result: %f", auto_corr[0]);
		iter = 0;
	}
	else{
		iter++;	
	}
}

/*
void estimate_pitch(){
	float* sample_ptr = &sample_history[(SAMPLE_HISTORY_LENGTH - 1) - AUTO_CORR_LENGTH];

	for(uint16_t i = 0; i < AUTO_CORR_LENGTH; i++){
		dsps_mul_f32(sample_ptr, sample_ptr + i, temp_array, AUTO_CORR_LENGTH - i, 1, 1, 1);

		auto_corr[i] = 0.0;
		for(uint16_t j = 0; j < AUTO_CORR_LENGTH - i; j++){
			auto_corr[i] += temp_array[j];
		}
	}

	// Normalize
	dsps_mulc_f32(auto_corr, auto_corr, AUTO_CORR_LENGTH, 1.0/auto_corr[0], 1, 1);

	static uint16_t iter = 0;
	if(iter == 100){
		ESP_LOGI(TAG, "Result: %f", auto_corr[0]);
		iter = 0;
	}
	else{
		iter++;	
	}
}
*/