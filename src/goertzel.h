// -----------------------------------------------------------------
//                                 _                  _       _
//                                | |                | |     | |
//    __ _    ___     ___   _ __  | |_   ____   ___  | |     | |__
//   / _` |  / _ \   / _ \ | '__| | __| |_  /  / _ \ | |     | '_ \ 
//  | (_| | | (_) | |  __/ | |    | |_   / /  |  __/ | |  _  | | | |
//   \__, |  \___/   \___| |_|     \__| /___|  \___| |_| (_) |_| |_|
//    __/ |
//   |___/
//
// Functions related to the computation and post-processing of the Goertzel Algorithm to compute DFT
// https://en.wikipedia.org/wiki/Goertzel_algorithm

#define TWOPI   6.28318530
#define FOURPI 12.56637061
#define SIXPI  18.84955593

#define NOISE_CALIBRATION_FRAMES 512

#define BOTTOM_NOTE 24	// THESE ARE IN QUARTER-STEPS, NOT HALF-STEPS! That's 24 notes to an octave
#define NOTE_STEP 2 // Use half-steps anyways

uint32_t noise_calibration_active_frames_remaining = 0;

const float notes[] = {
	55.0, 56.635235, 58.27047, 60.00294, 61.73541, 63.5709, 65.40639, 67.351025, 69.29566, 71.355925, 73.41619, 75.59897, 77.78175, 80.09432, 82.40689, 84.856975, 87.30706, 89.902835, 92.49861, 95.248735, 97.99886, 100.91253, 103.8262, 106.9131, 110.0, 113.27045, 116.5409, 120.00585, 123.4708, 127.1418, 130.8128, 134.70205, 138.5913, 142.71185, 146.8324, 151.19795, 155.5635, 160.18865, 164.8138, 169.71395, 174.6141, 179.80565, 184.9972, 190.49745, 195.9977, 201.825, 207.6523, 213.82615, 220.0, 226.54095, 233.0819, 240.0118, 246.9417, 254.28365, 261.6256, 269.4041, 277.1826, 285.4237, 293.6648, 302.3959, 311.127, 320.3773, 329.6276, 339.4279, 349.2282, 359.6113, 369.9944, 380.9949, 391.9954, 403.65005, 415.3047, 427.65235, 440.0, 453.0819, 466.1638, 480.02355, 493.8833, 508.5672, 523.2511, 538.8082, 554.3653, 570.8474, 587.3295, 604.79175, 622.254, 640.75455, 659.2551, 678.8558, 698.4565, 719.22265, 739.9888, 761.98985, 783.9909, 807.30015, 830.6094, 855.3047, 880.0, 906.16375, 932.3275, 960.04705, 987.7666, 1017.1343, 1046.502, 1077.6165, 1108.731, 1141.695, 1174.659, 1209.5835, 1244.508, 1281.509, 1318.51, 1357.7115, 1396.913, 1438.4455, 1479.978, 1523.98, 1567.982, 1614.6005, 1661.219, 1710.6095, 1760.0, 1812.3275, 1864.655, 1920.094, 1975.533, 2034.269, 2093.005, 2155.233, 2217.461, 2283.3895, 2349.318, 2419.167, 2489.016, 2563.018, 2637.02, 2715.4225, 2793.825, 2876.8905, 2959.956, 3047.96, 3135.964, 3229.2005, 3322.437, 3421.2185, 3520.0, 3624.655, 3729.31, 3840.1875, 3951.065, 4068.537, 4186.009, 4310.4655, 4434.922, 4566.779, 4698.636, 4838.334, 4978.032, 5126.0365, 5274.041, 5430.8465, 5587.652, 5753.7815, 5919.911, 6095.919, 6271.927, 6458.401, 6644.875, 6842.4375, 7040.0, 7249.31, 7458.62, 7680.375, 7902.13, 8137.074, 8372.018, 8620.931, 8869.844, 9133.558, 9397.272, 9676.668, 9956.064, 10252.072, 10548.08, 10861.69, 11175.3, 11507.56, 11839.82, 12191.835, 12543.85, 12916.8, 13289.75, 13684.875, 14080.0, 14498.62, 14917.24, 15360.75, 15804.26, 16274.145, 16744.03, 17241.855, 17739.68, 18267.11, 18794.54, 19353.36, 19912.18, 20504.17, 21096.16, 21723.38, 22350.6, 23015.12, 23679.64, 24383.67, 25087.7, 25833.6, 26579.5, 27369.75, 28160.0, 28997.24, 29834.48, 30721.5, 31608.52, 32548.295, 33488.07, 34483.72, 35479.37, 36534.225, 37589.08, 38706.665, 39824.25, 41008.285, 42192.32, 43446.76, 44701.2, 46030.24, 47359.28, 48767.34, 50175.4, 51667.2};

float full_spectrum_frequencies[64] = {
	50.0, 150.79, 251.59, 352.38, 453.17, 553.97, 654.76, 755.56,
	856.35, 957.14, 1057.94, 1158.73, 1259.52, 1360.32, 1461.11, 1561.90,
	1662.70, 1763.49, 1864.29, 1965.08, 2065.87, 2166.67, 2267.46, 2368.25,
	2469.05, 2569.84, 2670.63, 2771.43, 2872.22, 2973.02, 3073.81, 3174.60,
	3275.40, 3376.19, 3476.98, 3577.78, 3678.57, 3779.37, 3880.16, 3980.95,
	4081.75, 4182.54, 4283.33, 4384.13, 4484.92, 4585.71, 4686.51, 4787.30,
	4888.10, 4988.89, 5089.68, 5190.48, 5291.27, 5392.06, 5492.86, 5593.65,
	5694.44, 5795.24, 5896.03, 5996.83, 6097.62, 6198.41, 6299.21, 6400.0};

float window_lookup[4096];

freq frequencies_musical[NUM_FREQS];
uint16_t max_goertzel_block_size = 0;

volatile bool magnitudes_locked = false;

float spectrogram[NUM_FREQS];
float chromagram[12];

const uint8_t NUM_SPECTROGRAM_AVERAGE_SAMPLES = 8;
float spectrogram_smooth[NUM_FREQS] = { 0.0 };
float spectrogram_average[NUM_SPECTROGRAM_AVERAGE_SAMPLES][NUM_FREQS];
uint8_t spectrogram_average_index = 0;

void init_goertzel(uint16_t frequency_slot, float frequency, float bandwidth) {
	// Calculate the block size based on the desired bandwidth
	frequencies_musical[frequency_slot].block_size = SAMPLE_RATE / (bandwidth);

	// Adjust the block size to be divisible by 4
	while (frequencies_musical[frequency_slot].block_size % 4 != 0) {
		frequencies_musical[frequency_slot].block_size -= 1;
	}

	// Limit the block size to the maximum sample history length
	if (frequencies_musical[frequency_slot].block_size > SAMPLE_HISTORY_LENGTH - 1) {
		frequencies_musical[frequency_slot].block_size = SAMPLE_HISTORY_LENGTH - 1;
	}

	// Update the maximum goertzel block size
	max_goertzel_block_size = max(max_goertzel_block_size, frequencies_musical[frequency_slot].block_size);

	// Calculate the window step size
	frequencies_musical[frequency_slot].window_step = 4096.0 / frequencies_musical[frequency_slot].block_size;

	// Calculate the coefficients for the goertzel algorithm
	float k = (int)(0.5 + ((frequencies_musical[frequency_slot].block_size * frequencies_musical[frequency_slot].target_freq) / SAMPLE_RATE));
	float w = (2.0 * PI * k) / frequencies_musical[frequency_slot].block_size;
	float cosine = cos(w);
	float sine = sin(w);
	frequencies_musical[frequency_slot].coeff = 2.0 * cosine;
}

void init_goertzel_constants_musical() {
	for (uint16_t i = 0; i < NUM_FREQS; i++) {
		// INIT MUSICAL FREQS
		uint16_t note = BOTTOM_NOTE + (i * NOTE_STEP);
		frequencies_musical[i].target_freq = notes[note];

		float neighbor_left;
		float neighbor_right;

		if (note == 0) {
			neighbor_left = notes[note];
			neighbor_right = notes[note + 1];
		}
		else if (note == NUM_FREQS - 1) {
			neighbor_left = notes[note - 1];
			neighbor_right = notes[note];
		}
		else {
			neighbor_left = notes[note - 1];
			neighbor_right = notes[note + 1];
		}

		float neighbor_distance_hz = max(
			fabs(frequencies_musical[i].target_freq - neighbor_left),
			fabs(frequencies_musical[i].target_freq - neighbor_right));

		init_goertzel(i, frequencies_musical[i].target_freq, neighbor_distance_hz * 4.0);
	}
}

void init_window_lookup() {
    float sigma = 0.8; // For gaussian window

    for (uint16_t i = 0; i < 2048; i++) {
        float ratio = i / 2047.0;

        float n_minus_halfN = i - 2048 / 2;
        float gaussian_weighing_factor = exp(-0.5 * pow((n_minus_halfN / (sigma * 2048 / 2)), 2));

        // Hamming window
        //float weighing_factor = 0.54 * (1.0 - cos(TWOPI * ratio));

        // Blackman-Harris window
        //float weighing_factor = 0.3635819 - (0.4891775 * cos(TWOPI * ratio)) + (0.1365995 * cos(FOURPI * ratio)) - (0.0106411 * cos(SIXPI * ratio));

        // Gaussian window
        float weighing_factor = gaussian_weighing_factor;

        window_lookup[i] = weighing_factor;
        window_lookup[4095 - i] = weighing_factor; // Mirror the value for the second half
    }
}

// Function to find the median in a small array of floats
float find_median(float* data, int size) {
	float temp;
	for (int i = 0; i < size - 1; i++) {
		for (int j = i + 1; j < size; j++) {
			if (data[i] > data[j]) {
				temp = data[i];
				data[i] = data[j];
				data[j] = temp;
			}
		}
	}
	return data[size / 2];
}

// In-place median filter function
void median_filter(float* spectrogram_column) {
	const uint16_t FILTER_SIZE = 3;

	float window[FILTER_SIZE];
	float sorted_window[FILTER_SIZE];
	float output[NUM_FREQS];

	for (int i = 0; i < NUM_FREQS; i++) {
		// Populate the window with data
		for (int j = 0; j < FILTER_SIZE; j++) {
			int index = i + j - FILTER_SIZE / 2;
			index = index < 0 ? 0 : (index >= NUM_FREQS ? NUM_FREQS - 1 : index);
			window[j] = spectrogram_column[index];
		}

		// Find median and store it in the output array
		memcpy(sorted_window, window, sizeof(window));
		output[i] = find_median(sorted_window, FILTER_SIZE);
	}

	// Copy the filtered data back into the original array
	memcpy(spectrogram_column, output, sizeof(output));
}

float calculate_magnitude_of_bin(uint16_t bin_number) {
	float normalized_magnitude;
	float scale;

	profile_function([&]() {
		float q0 = 0;
		float q1 = 0;
		float q2 = 0;
		float window_pos = 0.0;

		const uint16_t block_size = frequencies_musical[bin_number].block_size;

		float coeff = frequencies_musical[bin_number].coeff;
		float window_step = frequencies_musical[bin_number].window_step;

		float* sample_ptr = &sample_history[(SAMPLE_HISTORY_LENGTH - 1) - block_size];

		for (uint16_t i = 0; i < block_size; i++) {
			float windowed_sample = sample_ptr[i] * window_lookup[uint32_t(window_pos)];
			q0 = coeff * q1 - q2 + windowed_sample;
			q2 = q1;
			q1 = q0;

			window_pos += window_step;
		}

		float magnitude_squared = (q1 * q1) + (q2 * q2) - q1 * q2 * coeff;
		float magnitude = sqrt(magnitude_squared);
		normalized_magnitude = magnitude_squared / (block_size / 2.0);

		float progress = float(bin_number) / NUM_FREQS;
		progress *= progress;
		progress *= progress;
		scale = (progress * 0.995) + 0.005;

	}, __func__ );

	return normalized_magnitude * scale;
}

float collect_and_filter_noise(float input_magnitude, uint16_t bin) {
	if (noise_calibration_active_frames_remaining == 0) {
		float output_magnitude = input_magnitude - noise_spectrum[bin];
		if (output_magnitude < 0.0) {
			output_magnitude = 0.0;
		}

		return output_magnitude;
	}
	else {
		if (input_magnitude > noise_spectrum[bin]) {
			noise_spectrum[bin] = input_magnitude*0.75;
		}

		return input_magnitude;
	}
}

void calculate_magnitudes() {
	profile_function([&]() {
		magnitudes_locked = true;

		const uint16_t NUM_AVERAGE_SAMPLES = 6;

		static bool interlacing_frame_field = 0;
		static float magnitudes_raw[NUM_FREQS];
		static float magnitudes_avg[NUM_AVERAGE_SAMPLES][NUM_FREQS];
		static float magnitudes_smooth[NUM_FREQS];
		static float max_val_smooth = 0.0;

		static uint32_t iter = 0;
		iter++;

		interlacing_frame_field = !interlacing_frame_field;

		float max_val = 0.0;
		// Iterate over all target frequencies
		for (uint16_t i = 0; i < NUM_FREQS; i++) {
			bool interlace_line = i % 2;
			if (interlace_line == interlacing_frame_field) {
				// Get raw magnitude of frequency
				magnitudes_raw[i] = calculate_magnitude_of_bin(i);  // fast_mode enabled (downsampled audio)
				magnitudes_raw[i] = collect_and_filter_noise(magnitudes_raw[i], i);
			}

			// Store raw magnitude
			frequencies_musical[i].magnitude_full_scale = magnitudes_raw[i];

			// Add raw magnitude to moving average array
			magnitudes_avg[iter % NUM_AVERAGE_SAMPLES][i] = magnitudes_raw[i];

			// Sum up current moving average and divide
			float magnitudes_avg_result = 0.0;
			for (uint8_t a = 0; a < NUM_AVERAGE_SAMPLES; a++) {
				magnitudes_avg_result += magnitudes_avg[a][i];
			}
			magnitudes_avg_result /= NUM_AVERAGE_SAMPLES;

			// Store averaged value
			magnitudes_smooth[i] = magnitudes_avg_result;

			// Accumulate maximum magnitude of all bins
			if (magnitudes_smooth[i] > max_val) {
				max_val = magnitudes_smooth[i];
			}
		}

		if(noise_calibration_active_frames_remaining > 0){
			// Not done yet? Decrement...
			noise_calibration_active_frames_remaining -= 1;

			// If background noise calibration just finished
			if(noise_calibration_active_frames_remaining == 0){
				// Let the UI know
				broadcast("noise_cal_ready");
				save_config();
				save_noise_spectrum();
				
			}
		}

		// Smooth max_val with different speed limits for increases vs. decreases
		if (max_val > max_val_smooth) {
			float delta = max_val - max_val_smooth;
			max_val_smooth += delta * 0.005;
		}
		if (max_val < max_val_smooth) {
			float delta = max_val_smooth - max_val;
			max_val_smooth -= delta * 0.005;
		}

		// Set a minimum "floor" to auto-range for, below this we don't auto-range anymore
		if (max_val_smooth < 0.000001) {
			max_val_smooth = 0.000001;
		}

		// Calculate auto-ranging scale
		float autoranger_scale = 1.0 / (max_val_smooth);

		// Iterate over all frequencies
		for (uint16_t i = 0; i < NUM_FREQS; i++) {
			// Apply the auto-scaler
			frequencies_musical[i].magnitude = clip_float(magnitudes_smooth[i] * autoranger_scale);
			spectrogram[i] = frequencies_musical[i].magnitude;
		}

		spectrogram_average_index++;
		if(spectrogram_average_index >= NUM_SPECTROGRAM_AVERAGE_SAMPLES){
			spectrogram_average_index = 0;
		}

		for(uint16_t i = 0; i < NUM_FREQS; i++){
			spectrogram_average[spectrogram_average_index][i] = spectrogram[i];

			spectrogram_smooth[i] = 0;
			for(uint16_t a = 0; a < NUM_SPECTROGRAM_AVERAGE_SAMPLES; a++){
				spectrogram_smooth[i] += spectrogram_average[a][i];
			}
			spectrogram_smooth[i] /= float(NUM_SPECTROGRAM_AVERAGE_SAMPLES);
		}

		magnitudes_locked = false;
	}, __func__ );
	___();
}

void start_noise_calibration() {
	Serial.println("Starting noise cal...");
	memset(noise_spectrum, 0, sizeof(float) * NUM_FREQS);
	configuration.vu_floor = 0.0;
	noise_calibration_active_frames_remaining = NOISE_CALIBRATION_FRAMES;
}

void get_chromagram(){
	memset(chromagram, 0, sizeof(float) * 12);

	float max_val = 0.2;
	for(uint16_t i = 0; i < 60; i++){
		chromagram[ i % 12 ] += (spectrogram_smooth[i] / 5.0);

		max_val = max(max_val, chromagram[ i % 12 ]);
	}

	float auto_scale = 1.0 / max_val;

	for(uint16_t i = 0; i < 12; i++){
		chromagram[i] *= auto_scale;
	}
}