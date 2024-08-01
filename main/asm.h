esp_err_t dsps_addc_f32_ae32_fast(const float *input, float *output, int len, float C, int step_in, int step_out);
esp_err_t dsps_mulc_f32_ae32_fast(const float *input, float *output, int len, float C, int step_in, int step_out);

esp_err_t dsps_add_f32_ae32_fast(const float *input1, const float *input2, float *output, int len, int step);
esp_err_t dsps_mul_f32_ae32_fast(const float *input1, const float *input2, float *output, int len, int step);

void test_asm(bool new_version){
	//ESP_LOGI(TAG, "########################################################");
	if(new_version == true){
		//ESP_LOGI(TAG, "NEW ASM TEST");
	}
	else{
		//ESP_LOGI(TAG, "OLD ASM TEST");
	}

	__attribute__((aligned(16)))
	float input[64] = {0.000};

	__attribute__((aligned(16)))
	float output[64] = {0.000};

	for(uint16_t i = 0; i < 64; i++){
		input[i]  = 0.1*i;
	}

	int64_t t_start = esp_timer_get_time();	
	if(new_version == true){
		for(uint16_t i = 0; i < 1000; i++){
			dsps_mulc_f32_ae32(input, output, 64, 2.0, 1, 1);
		}
	}
	else{
		for(uint16_t i = 0; i < 1000; i++){
			dsps_mulc_f32_ae32(input, output, 64, 2.0, 1, 1);
		}
	}
	int64_t t_end = esp_timer_get_time();

	//ESP_LOGI(TAG, "Microseconds to run 1,000 times: %lli", (t_end - t_start));
	//ESP_LOGI(TAG, "Output: %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f...", output[0], output[1], output[2], output[3], output[4], output[5], output[6], output[7], output[8], output[9]);
	//ESP_LOGI(TAG, "########################################################");
}