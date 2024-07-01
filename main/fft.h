#define FFT_SIZE 256
#define NUM_FFT_AVERAGE_SAMPLES 4

// FFT Stuffs
__attribute__((aligned(16)))
float fft_input[FFT_SIZE];

__attribute__((aligned(16)))
float fft_window[FFT_SIZE];

__attribute__((aligned(16)))
float fft_input_complex[FFT_SIZE<<1];

__attribute__((aligned(16)))
float fft_output[FFT_SIZE];

__attribute__((aligned(16)))
float fft_max[FFT_SIZE];

__attribute__((aligned(16)))
float fft_smooth[1 + NUM_FFT_AVERAGE_SAMPLES][FFT_SIZE]; // One slot for output, others for averaging
uint16_t fft_averaging_index = 1;

void init_fft(){
	dsps_fft4r_init_fc32(NULL, FFT_SIZE << 1);
	dsps_wind_blackman_nuttall_f32(fft_window, FFT_SIZE);
}

void perform_fft(){
	memset(fft_input_complex, 0, sizeof(float) * (FFT_SIZE << 1));

	const uint8_t step_size = 3;
	for(uint16_t i = 0; i < FFT_SIZE; i++){
		fft_input[i] = sample_history[((SAMPLE_HISTORY_LENGTH-1) - (FFT_SIZE*step_size)) + i*step_size ];
	}

	dsps_mul_f32(fft_input, fft_window, fft_input, FFT_SIZE, 1, 1, 1);

	for(uint16_t i = 0; i < FFT_SIZE; i++){
		fft_input_complex[i*2+0] = fft_input[i];
		fft_input_complex[i*2+1] = 0.0;
	}

	dsps_fft4r_fc32(fft_input_complex, FFT_SIZE);
	dsps_bit_rev4r_fc32(fft_input_complex, FFT_SIZE);
	// Convert one complex vector with length N >> 1 to one real spectrum vector with length N >> 1
    dsps_cplx2real_fc32(fft_input_complex, FFT_SIZE);

	// Calculate the magnitude of the complex numbers and convert to 0.0 to 1.0 range
	for (uint16_t i = 0 ; i < FFT_SIZE; i++) {
		float progress = (float)i / FFT_SIZE;
		float real = fft_input_complex[i << 1];
		float imag = fft_input_complex[(i << 1) + 1];
		fft_output[i] = sqrtf(real * real + imag * imag) / (FFT_SIZE >> 1);
		fft_output[i] = fft_output[i]*(0.5+0.5*progress);

		fft_max[i] = fmaxf(fft_max[i], fft_output[i]);
	}

	memcpy(fft_smooth[ fft_averaging_index ], fft_output, sizeof(float) * FFT_SIZE);
	fft_averaging_index += 1;
	if(fft_averaging_index >= NUM_FFT_AVERAGE_SAMPLES + 1){
		fft_averaging_index = 1;
	}

	memset(fft_smooth[0], 0, sizeof(float) * FFT_SIZE);
	for(uint8_t i = 1; i < NUM_FFT_AVERAGE_SAMPLES + 1; i++){
		dsps_add_f32(fft_smooth[0], fft_smooth[i], fft_smooth[0], FFT_SIZE, 1, 1, 1);
	}

	dsps_mulc_f32_ansi(fft_smooth[0], fft_smooth[0], FFT_SIZE, 1.0 / NUM_FFT_AVERAGE_SAMPLES, 1, 1);

	
}