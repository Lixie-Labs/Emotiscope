// -----------------------------------------------------
//   _                                            _
//  | |                                          | |
//  | |_    ___   _ __ ___    _ __     ___       | |__
//  | __|  / _ \ | '_ ` _ \  | '_ \   / _ \      | '_ \ 
//  | |_  |  __/ | | | | | | | |_) | | (_) |  _  | | | |
//   \__|  \___| |_| |_| |_| | .__/   \___/  (_) |_| |_|
//                           | |
//                           |_|
//
// Functions related to the computation of possible tempi in music

//--------------------------------------------

#define NOVELTY_HISTORY_LENGTH (1024)  // 50 FPS for 20.48 seconds
#define NOVELTY_LOG_HZ (50)

#define TEMPO_LOW (64-16)
#define TEMPO_HIGH (192-16)

#define BEAT_SHIFT_PERCENT (0.00)

#define NUM_TEMPI (64)

bool silence_detected = true;
float silence_level = 1.0;

float tempo_confidence = 0.0;

float MAX_TEMPO_RANGE = 1.0;

float tempi_bpm_values_hz[NUM_TEMPI];

float novelty_curve[NOVELTY_HISTORY_LENGTH];
float novelty_curve_normalized[NOVELTY_HISTORY_LENGTH];

float vu_curve[NOVELTY_HISTORY_LENGTH];

tempo tempi[NUM_TEMPI];
float tempi_smooth[NUM_TEMPI];
float tempi_power_sum = 0.0;

uint16_t find_closest_tempo_bin(float target_bpm) {
	float target_bpm_hz = target_bpm / 60.0;

	float smallest_difference = 10000000.0;
	uint16_t smallest_difference_index = 0;

	for (uint16_t i = 0; i < NUM_TEMPI; i++) {
		float difference = fabs(target_bpm_hz - tempi_bpm_values_hz[i]);
		if (difference < smallest_difference) {
			smallest_difference = difference;
			smallest_difference_index = i;
		}
	}

	return smallest_difference_index;
}

void init_tempo_goertzel_constants() {
	for (uint16_t i = 0; i < NUM_TEMPI; i++) {
		float progress = float(i) / NUM_TEMPI;
		float tempi_range = TEMPO_HIGH - TEMPO_LOW;
		float tempo = tempi_range * progress + TEMPO_LOW;

		tempi_bpm_values_hz[i] = tempo / 60.0;
		//Serial.print("TEMPO HZ:");
		//Serial.println(tempi_bpm_values_hz[i]);
	}

	for (uint16_t i = 0; i < NUM_TEMPI; i++) {
		tempi[i].target_tempo_hz = tempi_bpm_values_hz[i];

		float neighbor_left;
		float neighbor_right;

		if (i == 0) {
			neighbor_left = tempi_bpm_values_hz[i];
			neighbor_right = tempi_bpm_values_hz[i + 1];
		}
		else if (i == NUM_TEMPI - 1) {
			neighbor_left = tempi_bpm_values_hz[i - 1];
			neighbor_right = tempi_bpm_values_hz[i];
		}
		else {
			neighbor_left = tempi_bpm_values_hz[i - 1];
			neighbor_right = tempi_bpm_values_hz[i + 1];
		}

		float neighbor_left_distance_hz = fabs(neighbor_left - tempi[i].target_tempo_hz);
		float neighbor_right_distance_hz = fabs(neighbor_right - tempi[i].target_tempo_hz);
		float max_distance_hz = 0;

		if (neighbor_left_distance_hz > max_distance_hz) {
			max_distance_hz = neighbor_left_distance_hz;
		}
		if (neighbor_right_distance_hz > max_distance_hz) {
			max_distance_hz = neighbor_right_distance_hz;
		}

		tempi[i].block_size = NOVELTY_LOG_HZ / (max_distance_hz*0.5);

		if (tempi[i].block_size > NOVELTY_HISTORY_LENGTH) {
			tempi[i].block_size = NOVELTY_HISTORY_LENGTH;
		}

		//Serial.print("TEMPI ");
		//Serial.print(i);
		//Serial.print(" BLOCK SIZE: ");
		//Serial.println(tempi[i].block_size);

		float k = (int)(0.5 + ((tempi[i].block_size * tempi[i].target_tempo_hz) / NOVELTY_LOG_HZ));
		float w = (2.0 * PI * k) / tempi[i].block_size;
		tempi[i].cosine = cos(w);
		tempi[i].sine = sin(w);
		tempi[i].coeff = 2.0 * tempi[i].cosine;

		tempi[i].window_step = 4096.0 / tempi[i].block_size;

		// tempi[i].target_tempo_hz *= 0.5;

		// float radians_per_second = (PI * (tempi[i].target_tempo_hz));
		tempi[i].phase_radians_per_reference_frame = ((2.0 * PI * tempi[i].target_tempo_hz) / float(REFERENCE_FPS));
	}
}

float unwrap_phase(float phase) {
	while (phase - phase > M_PI) {
		phase -= 2 * M_PI;
	}
	while (phase - phase < -M_PI) {
		phase += 2 * M_PI;
	}

	return phase;
}

float calculate_magnitude_of_tempo(uint16_t tempo_bin) {
	float normalized_magnitude;

	profile_function([&]() {
		uint16_t block_size = tempi[tempo_bin].block_size;

		float q1 = 0;
		float q2 = 0;

		float window_pos = 0.0;

		for (uint16_t i = 0; i < block_size; i++) {
			float progress = float(i) / block_size;
			float sample_novelty = novelty_curve_normalized[((NOVELTY_HISTORY_LENGTH - 1) - block_size) + i];
			float sample_vu      =                 vu_curve[((NOVELTY_HISTORY_LENGTH - 1) - block_size) + i];
			float sample = (sample_novelty + sample_vu) / 2.0;

			float q0 = tempi[tempo_bin].coeff * q1 - q2 + (sample * window_lookup[uint32_t(window_pos)]);
			q2 = q1;
			q1 = q0;

			window_pos += tempi[tempo_bin].window_step;
		}

		float real = (q1 - q2 * tempi[tempo_bin].cosine);
		float imag = (q2 * tempi[tempo_bin].sine);

		// Calculate phase
		tempi[tempo_bin].phase_target = (unwrap_phase(atan2(imag, real)) + (PI * BEAT_SHIFT_PERCENT));
		if (tempi[tempo_bin].phase_target > PI) {
			tempi[tempo_bin].phase_target -= (2 * PI);
		}
		else if (tempi[tempo_bin].phase_target < -PI) {
			tempi[tempo_bin].phase_target += (2 * PI);
		}

		tempi[tempo_bin].phase = tempi[tempo_bin].phase_target;

		float magnitude_squared = (q1 * q1) + (q2 * q2) - q1 * q2 * tempi[tempo_bin].coeff;
		float magnitude = sqrt(magnitude_squared);
		normalized_magnitude = magnitude / (block_size / 2.0);

		float progress = 1.0 - (tempo_bin / float(NUM_TEMPI));
		progress *= progress;

		float scale = (0.5 * progress) + 0.5;

		normalized_magnitude *= scale;
	}, __func__ );

	return normalized_magnitude;
}

void calculate_tempi_magnitudes(int16_t single_bin = -1) {
	profile_function([&]() {
		float max_val = 0.0;
		for (uint16_t i = 0; i < NUM_TEMPI; i++) {
			// Should we update all tempo magnitudes on every frame or just one on all frames?
			if (single_bin != -1) {
				if (i == single_bin) {
					tempi[i].magnitude_full_scale = calculate_magnitude_of_tempo(single_bin);
				}
			}
			else {
				tempi[i].magnitude_full_scale = calculate_magnitude_of_tempo(i);
			}

			if (tempi[i].magnitude_full_scale > max_val) {
				max_val = tempi[i].magnitude_full_scale;
			}
		}

		if (max_val < 0.004) {
			max_val = 0.004;
		}

		float autoranger_scale = 1.0 / (max_val);

		for (uint16_t i = 0; i < NUM_TEMPI; i++) {
			float scaled_magnitude = (tempi[i].magnitude_full_scale * autoranger_scale);
			if (scaled_magnitude < 0.0) {
				scaled_magnitude = 0.0;
			}
			if (scaled_magnitude > 1.0) {
				scaled_magnitude = 1.0;
			}

			tempi[i].magnitude = scaled_magnitude * scaled_magnitude * scaled_magnitude;
		}
	}, __func__ );
}

void normalize_novelty_curve_slow() {
	static float max_val = 0.00001;
	static float max_val_smooth = 0.1;

	max_val *= 0.99;
	for (uint16_t i = 0; i < NOVELTY_HISTORY_LENGTH; i += 4) {
		max_val = max(max_val, novelty_curve[i + 0]);
		max_val = max(max_val, novelty_curve[i + 1]);
		max_val = max(max_val, novelty_curve[i + 2]);
		max_val = max(max_val, novelty_curve[i + 3]);
	}
	max_val_smooth = max(0.1f, max_val_smooth * 0.99f + max_val * 0.01f);

	float auto_scale = 1.0 / max_val_smooth;

	for (uint16_t i = 0; i < NOVELTY_HISTORY_LENGTH; i += 4) {
		novelty_curve_normalized[i + 0] = clip_float(novelty_curve[i + 0] * auto_scale);
		novelty_curve_normalized[i + 1] = clip_float(novelty_curve[i + 1] * auto_scale);
		novelty_curve_normalized[i + 2] = clip_float(novelty_curve[i + 2] * auto_scale);
		novelty_curve_normalized[i + 3] = clip_float(novelty_curve[i + 3] * auto_scale);
	}
}

void normalize_novelty_curve() {
	profile_function([&]() {
		static float max_val = 0.00001;
		static float max_val_smooth = 0.1;

		max_val *= 0.99;
		for (uint16_t i = 0; i < NOVELTY_HISTORY_LENGTH; i += 4) {
			max_val = max(max_val, novelty_curve[i + 0]);
			max_val = max(max_val, novelty_curve[i + 1]);
			max_val = max(max_val, novelty_curve[i + 2]);
			max_val = max(max_val, novelty_curve[i + 3]);
		}
		max_val_smooth = max(0.1f, max_val_smooth * 0.99f + max_val * 0.01f);

		float auto_scale = 1.0 / max_val;
		dsps_mulc_f32(novelty_curve, novelty_curve_normalized, NOVELTY_HISTORY_LENGTH, auto_scale, 1, 1);
	}, __func__ );
}

void update_tempo() {
	profile_function([&]() {
		static uint32_t iter = 0;
		iter++;

		normalize_novelty_curve();

		if (iter % 2 == 0) {
			static uint16_t calc_bin = 0;

			uint16_t max_bin = (NUM_TEMPI - 1) * MAX_TEMPO_RANGE;

			calculate_tempi_magnitudes(calc_bin);
			calc_bin++;
			if (calc_bin >= max_bin) {
				calc_bin = 0;
			}
		}
	}, __func__ );
}

void log_novelty(float input) {
	shift_array_left(novelty_curve, NOVELTY_HISTORY_LENGTH, 1);
	novelty_curve[NOVELTY_HISTORY_LENGTH - 1] = input;
}

void log_vu(float input) {
	shift_array_left(vu_curve, NOVELTY_HISTORY_LENGTH, 1);
	vu_curve[NOVELTY_HISTORY_LENGTH - 1] = input;
}

void reduce_tempo_history(float reduction_amount) {
	float reduction_amount_inv = 1.0 - reduction_amount;

	for (uint16_t i = 0; i < NOVELTY_HISTORY_LENGTH; i++) {
		novelty_curve[i] = max(novelty_curve[i] * reduction_amount_inv, 0.00001f);	// never go full zero
		vu_curve[i]      = max(     vu_curve[i] * reduction_amount_inv, 0.00001f);
	}
}

void check_silence(float current_novelty) {
	float min_val = 1.0;
	float max_val = 0.0;
	for (uint16_t i = 0; i < 128; i++) {
		float recent_novelty = novelty_curve_normalized[(NOVELTY_HISTORY_LENGTH - 1 - 128) + i];
		recent_novelty = min(0.5f, recent_novelty) * 2.0;

		float scaled_value = sqrt(recent_novelty);
		max_val = max(max_val, scaled_value);
		min_val = min(min_val, scaled_value);
	}
	float novelty_contrast = fabs(max_val - min_val);
	float silence_level_raw = 1.0 - novelty_contrast;

	silence_level = max(0.0f, silence_level_raw - 0.5f) * 2.0;
	if (silence_level_raw > 0.5) {
		silence_detected = true;
		reduce_tempo_history(silence_level * 0.10);
	}
	else {
		silence_level = 0.0;
		silence_detected = false;
	}

	// rendered_debug_value = silence_level;
}

void update_novelty() {
	static uint32_t next_update = t_now_us;

	const float update_interval_hz = NOVELTY_LOG_HZ;
	const uint32_t update_interval_us = 1000000 / update_interval_hz;

	if (t_now_us >= next_update) {
		next_update += update_interval_us;

		static uint32_t iter = 0;
		iter++;

		float current_novelty = 0.0;
		for (uint16_t i = 0; i < NUM_FREQS; i++) {
			float new_mag = spectrogram_smooth[i];	 // sqrt(sqrt(frequencies[i].magnitude));
			frequencies_musical[i].novelty = max(0.0f, new_mag - frequencies_musical[i].magnitude_last);
			frequencies_musical[i].magnitude_last = new_mag;

			current_novelty += frequencies_musical[i].novelty;
		}
		current_novelty /= float(NUM_FREQS);

		check_silence(current_novelty);

		log_novelty(log(1.0 + current_novelty));

		log_vu(vu_max);
		vu_max = 0.000001;
	}
}

void sync_beat_phase(uint16_t tempo_bin, float delta) {
	float push = (tempi[tempo_bin].phase_radians_per_reference_frame * delta);

	tempi[tempo_bin].phase += push;
	tempi[tempo_bin].phase_target += push;

	if (tempi[tempo_bin].phase > PI) {
		tempi[tempo_bin].phase -= (2 * PI);
	}
	else if (tempi[tempo_bin].phase < -PI) {
		tempi[tempo_bin].phase += (2 * PI);
	}

	if (tempi[tempo_bin].phase_target > PI) {
		tempi[tempo_bin].phase_target -= (2 * PI);
	}
	else if (tempi[tempo_bin].phase_target < -PI) {
		tempi[tempo_bin].phase_target += (2 * PI);
	}

	float walk_divider = 200.0;
	float sync_distance = fabs(tempi[tempo_bin].phase - tempi[tempo_bin].phase_target);
	if (tempi[tempo_bin].phase > tempi[tempo_bin].phase_target) {
		tempi[tempo_bin].phase -= (sync_distance / walk_divider);
	}
	else if (tempi[tempo_bin].phase < tempi[tempo_bin].phase_target) {
		tempi[tempo_bin].phase += (sync_distance / walk_divider);
	}

	tempi[tempo_bin].beat = sin(tempi[tempo_bin].phase);
}

void update_tempi_phase(float delta) {
	tempi_power_sum = 0.00000001;
	// Iterate over all tempi to smooth them and calculate the power sum
	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float progress = float(tempo_bin) / NUM_TEMPI;

		// Load the magnitude
		float tempi_magnitude = tempi[tempo_bin].magnitude;

		// Smooth it
		tempi_smooth[tempo_bin] = tempi_smooth[tempo_bin] * 0.975 + (tempi_magnitude) * 0.025;
		tempi_power_sum += tempi_smooth[tempo_bin];

		sync_beat_phase(tempo_bin, delta);
	}

	// Measure contribution factor of each tempi, calculate confidence level
	float max_contribution = 0.000001;
	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float contribution = tempi_smooth[tempo_bin] / tempi_power_sum;
		max_contribution = max(contribution, max_contribution);
	}

	tempo_confidence = max_contribution;
}