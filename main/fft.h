#define FFT_SIZE 256
#define NUM_FFT_AVERAGE_SAMPLES 4

// FFT Stuffs
__attribute__((aligned(16)))
float fft_input[FFT_SIZE];

__attribute__((aligned(16)))
float fft_input_filtered[FFT_SIZE];

__attribute__((aligned(16)))
float fft_window[FFT_SIZE];

__attribute__((aligned(16)))
float fft_input_complex[FFT_SIZE<<1];

__attribute__((aligned(16)))
float fft_max[FFT_SIZE];

__attribute__((aligned(16)))
float fft_smooth[1 + NUM_FFT_AVERAGE_SAMPLES][FFT_SIZE>>1]; // One slot [0] for output, others as input for averaging
uint16_t fft_averaging_index = 1;

//const float fft_high_frequency = 1000.0; 
const float fft_high_q_factor = 0.7;
float fft_high_coeffs_lpf[5];
float fft_high_w_lpf[5] = {0, 0};

void init_fft_high_filter(){
	esp_err_t ret = ESP_OK;
    
    ret = dsps_biquad_gen_lpf_f32(fft_high_coeffs_lpf, 0.25, fft_high_q_factor);
    if (ret  != ESP_OK) {
        //ESP_LOGE(TAG, "Operation error = %i", ret);
        return;
    }
}

void init_fft(){
	init_fft_high_filter();
	dsps_fft4r_init_fc32(NULL, FFT_SIZE << 1);
	dsps_wind_hann_f32(fft_window, FFT_SIZE);
}

void perform_fft(){
	start_profile(__COUNTER__, __func__);
	dsps_memset_aes3(fft_input_complex, 0, sizeof(float) * (FFT_SIZE << 1));

	for(uint16_t i = 0; i < FFT_SIZE; i++){
		fft_input[i] = sample_history_half_rate[
			(
				((SAMPLE_HISTORY_LENGTH)-1) - (FFT_SIZE)
			)
			+
			i
		];
	}

	// LPF
	//dsps_biquad_f32(fft_input, fft_input_filtered, FFT_SIZE, fft_high_coeffs_lpf, fft_high_w_lpf);
	dsps_memcpy_aes3(fft_input_filtered, fft_input, sizeof(float)*FFT_SIZE);

	static uint8_t step = 0;
	step++;
	int64_t start_time = esp_timer_get_time();
	dsps_mul_f32_ae32_fast(fft_input_filtered, fft_window, fft_input_filtered, FFT_SIZE, 1);

	for(uint16_t i = 0; i < FFT_SIZE; i+=1){
		fft_input_complex[ (i<<1) + 0 ] = fft_input_filtered[i+0];
	}

	dsps_fft4r_fc32(fft_input_complex, FFT_SIZE);
	dsps_bit_rev4r_fc32(fft_input_complex, FFT_SIZE);

	// Convert one complex vector with length N >> 1 to one real spectrum vector with length N >> 1
    dsps_cplx2real_fc32(fft_input_complex, FFT_SIZE);

	// Square the whole array to avoid doing it later
	dsps_mul_f32_ae32_fast(fft_input_complex, fft_input_complex, fft_input_complex, FFT_SIZE, 1);

	// Calculate the magnitude of the complex numbers and convert to 0.0 to 1.0 range
	for (uint16_t i = 0 ; i < (FFT_SIZE >> 1); i++) {
		float real = fft_input_complex[(i << 1) + 0];
		float imag = fft_input_complex[(i << 1) + 1];

		fft_smooth[fft_averaging_index][i] = real + imag;
	}

	// sqrtf the whole array at once
	dsps_sqrt_f32(fft_smooth[fft_averaging_index], fft_smooth[fft_averaging_index], (FFT_SIZE >> 1));

	// Multiply by 1 / (FFT_SIZE >> 1)
	dsps_mulc_f32_ae32_fast(fft_smooth[fft_averaging_index], fft_smooth[fft_averaging_index], (FFT_SIZE >> 1), 1.0 / (FFT_SIZE >> 1), 1, 1);

	float step_size_f = 1.0 / (FFT_SIZE>>1);
	float step_pos = 0.0;
	for (uint16_t i = 0 ; i < (FFT_SIZE >> 1); i++) {
		fft_smooth[fft_averaging_index][i] = fft_smooth[fft_averaging_index][i] * (0.5 + 0.5*step_pos);
		step_pos += step_size_f;
	}

	int64_t end_time = esp_timer_get_time();
	int64_t duration = end_time - start_time;
	if(step == 0){ ESP_LOGI(TAG, "FFT: %lld us", duration); }

	fft_averaging_index += 1;
	if(fft_averaging_index >= NUM_FFT_AVERAGE_SAMPLES + 1){
		fft_averaging_index = 1;
	}

	dsps_memset_aes3(fft_smooth[0], 0, sizeof(float) * (FFT_SIZE >> 1));
	for(uint8_t i = 1; i < NUM_FFT_AVERAGE_SAMPLES + 1; i++){
		dsps_add_f32(fft_smooth[0], fft_smooth[i], fft_smooth[0], FFT_SIZE>>1, 1, 1, 1);
	}

	for(uint16_t i = 0; i < 16; i++){
		float progress = (float)i / 16.0;
		fft_smooth[0][i] *= progress;
		fft_smooth[0][i] *= progress;
	}

	float max_val = 0.001;
	for(uint16_t i = 0; i < FFT_SIZE>>1; i++){
		max_val = fmaxf(max_val, fft_smooth[0][i]);
	}
	float auto_scale = 1.0 / max_val;
	static float auto_scale_smooth = 1.0;

	auto_scale_smooth = auto_scale_smooth * 0.6 + auto_scale * 0.4;

	dsps_mulc_f32_ae32_fast(fft_smooth[0], fft_smooth[0], FFT_SIZE>>1, auto_scale_smooth, 1, 1);

	for(uint16_t i = 0; i < FFT_SIZE>>1; i+=4){
		fft_smooth[0][i+0] = clip_float(fft_smooth[0][i+0]);
		fft_smooth[0][i+1] = clip_float(fft_smooth[0][i+1]);
		fft_smooth[0][i+2] = clip_float(fft_smooth[0][i+2]);
		fft_smooth[0][i+3] = clip_float(fft_smooth[0][i+3]);
	}

	// square it
	dsps_mul_f32_ae32_fast(fft_smooth[0], fft_smooth[0], fft_smooth[0], FFT_SIZE>>1, 1);

	for(uint16_t i = 0; i < FFT_SIZE>>1; i++){
		fft_max[i] = fmaxf(fft_max[i], fft_smooth[0][i]);
	}

	end_profile();
}