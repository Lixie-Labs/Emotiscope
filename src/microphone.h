// -----------------------------------------------------------------------------------------
//              _                                 _                                  _
//             (_)                               | |                                | |
//  _ __ ___    _    ___   _ __    ___    _ __   | |__     ___    _ __     ___      | |__
// | '_ ` _ \  | |  / __| | '__|  / _ \  | '_ \  | '_ \   / _ \  | '_ \   / _ \     | '_ \ 
// | | | | | | | | | (__  | |    | (_) | | |_) | | | | | | (_) | | | | | |  __/  _  | | | |
// |_| |_| |_| |_|  \___| |_|     \___/  | .__/  |_| |_|  \___/  |_| |_|  \___| (_) |_| |_|
//                                       | |
//                                       |_|
//
// Functions for reading and storing data acquired by the I2S microphone0

// Define I2S pins
#define I2S_BCLK_PIN 6
#define I2S_LRCLK_PIN 5
#define I2S_DIN_PIN 4

#define CHUNK_SIZE 64
#define SAMPLE_RATE 12800

#define SAMPLE_HISTORY_LENGTH 4096

float sample_history[SAMPLE_HISTORY_LENGTH];
const float recip_scale = 1.0 / 80000000.0;

volatile bool waveform_locked = false;
volatile bool waveform_sync_flag = false;

// TODO: Get SPH0645 microphone tested/working
void init_i2s_microphone() {
	// Configure I2S
	i2s_config_t i2s_config = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = SAMPLE_RATE,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
		.dma_buf_count = 4,
		.dma_buf_len = CHUNK_SIZE};

	i2s_pin_config_t pin_config = {
		.bck_io_num = I2S_BCLK_PIN,
		.ws_io_num = I2S_LRCLK_PIN,
		.data_out_num = I2S_PIN_NO_CHANGE,
		.data_in_num = I2S_DIN_PIN};

	i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

	i2s_set_pin(I2S_NUM_0, &pin_config);
}

void acquire_sample_chunk() {
	profile_function([&]() {
		// Buffer to hold audio samples
		int32_t new_samples_raw[CHUNK_SIZE];
		float new_samples_raw_float[CHUNK_SIZE];
		float new_samples[CHUNK_SIZE];

		// Read audio samples into int32_t buffer
		size_t bytes_read = 0;
		i2s_read(I2S_NUM_0, new_samples_raw, sizeof(new_samples_raw), &bytes_read, portMAX_DELAY);

		// Clip the sample value if it's too large, cast to large floats
		for (uint16_t i = 0; i < CHUNK_SIZE; i+=4) {
			new_samples_raw_float[i+0] = min(max((int32_t)new_samples_raw[i+0], (int32_t)-80000000), (int32_t)80000000);
			new_samples_raw_float[i+1] = min(max((int32_t)new_samples_raw[i+1], (int32_t)-80000000), (int32_t)80000000);
			new_samples_raw_float[i+2] = min(max((int32_t)new_samples_raw[i+2], (int32_t)-80000000), (int32_t)80000000);
			new_samples_raw_float[i+3] = min(max((int32_t)new_samples_raw[i+3], (int32_t)-80000000), (int32_t)80000000);
		}

		// Convert audio from large float range to -1.0 to 1.0 range
		dsps_mulc_f32(new_samples_raw_float, new_samples, CHUNK_SIZE, recip_scale, 1, 1);

		// Add new chunk to audio history
		waveform_locked = true;
		shift_and_copy_arrays(sample_history, SAMPLE_HISTORY_LENGTH, new_samples, CHUNK_SIZE);
		waveform_locked = false;
		waveform_sync_flag = true;

		/*
		// Used to print raw microphone values over UART for debugging
		float* samples = &sample_history[SAMPLE_HISTORY_LENGTH-1-CHUNK_SIZE];
		char output[32];
		memset(output, 0, 32);
		for(uint16_t i = 0; i < CHUNK_SIZE; i++){
			uint16_t offset = 16;
			uint16_t position = offset + int16_t(samples[i]*8.0);
			memset(output, 0, 32);
			memset(output, ' ', position);
			output[position] = '|';
			printf("%s\n", output);
		}
		*/

	}, __func__);
}
