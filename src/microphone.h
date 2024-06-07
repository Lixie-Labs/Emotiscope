/*
-----------------------------------------------------------------------------------------
             _                                 _                                  _
            (_)                               | |                                | |
 _ __ ___    _    ___   _ __    ___    _ __   | |__     ___    _ __     ___      | |__
| '_ ` _ \  | |  / __| | '__|  / _ \  | '_ \  | '_ \   / _ \  | '_ \   / _ \     | '_ \ 
| | | | | | | | | (__  | |    | (_) | | |_) | | | | | | (_) | | | | | |  __/  _  | | | |
|_| |_| |_| |_|  \___| |_|     \___/  | .__/  |_| |_|  \___/  |_| |_|  \___| (_) |_| |_|
                                      | |
                                      |_|

Functions for reading and storing data acquired by the I2S microphone
*/

#include "driver/i2s_std.h"
#include "driver/gpio.h"

// Define I2S pins
#define I2S_LRCLK_PIN 35
#define I2S_BCLK_PIN  36
#define I2S_DIN_PIN   37

#define CHUNK_SIZE 64
#define SAMPLE_RATE 12800*2

#define SAMPLE_HISTORY_LENGTH 4096

float sample_history[SAMPLE_HISTORY_LENGTH];
const float recip_scale = 1.0 / 131072.0; // max 18 bit signed value

volatile bool waveform_locked = false;
volatile bool waveform_sync_flag = false;

i2s_chan_handle_t rx_handle;

void init_i2s_microphone(){
	// Get the default channel configuration
	i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);

	// Create a new RX channel and get the handle of this channel
	i2s_new_channel(&chan_cfg, NULL, &rx_handle);

	// Configuration for the I2S standard mode, suitable for the SPH0645 microphone
	i2s_std_config_t std_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE), // BCLK frequency for a 2kHz sample rate (64 * 2kHz)
		.slot_cfg = {
			.data_bit_width = I2S_DATA_BIT_WIDTH_32BIT, // Data bit width as 24 bits
			.slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT, // Slot width as 32 bits to accommodate data
			.slot_mode = I2S_SLOT_MODE_STEREO, // Mono mode since it's a single microphone
			.slot_mask = I2S_STD_SLOT_RIGHT, // Only reading the left channel slot
			.ws_width = 32, // WS signal width as 32 BCLK periods (since BCLK/64 and we are in mono mode)
			.ws_pol = true, // Inverting WS polarity, so it changes on falling edge of BCLK
			.bit_shift = false, // No bit shift needed as MSB is delayed by 1 BCLK after WS
			.left_align = true, // Data is left-aligned within the 32-bit slot
			.big_endian = false, // Data format is little endian
			.bit_order_lsb = false, // MSB is received first
		},
		.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED,
			.bclk = (gpio_num_t)I2S_BCLK_PIN,
			.ws = (gpio_num_t)I2S_LRCLK_PIN,
			.dout = I2S_GPIO_UNUSED,
			.din = (gpio_num_t)I2S_DIN_PIN,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv = false,
			},
		},
	};



	// Initialize the channel
	i2s_channel_init_std_mode(rx_handle, &std_cfg);

	// Start the RX channel
	i2s_channel_enable(rx_handle);
}

void acquire_sample_chunk() {
	profile_function([&]() {
		// Buffer to hold audio samples
		uint32_t new_samples_raw[CHUNK_SIZE*2];
		float new_samples[CHUNK_SIZE*2];

		// Read audio samples into int32_t buffer, but **only when emotiscope is active**
		if( EMOTISCOPE_ACTIVE == true ){
			size_t bytes_read = 0;
			i2s_channel_read(rx_handle, new_samples_raw, CHUNK_SIZE*2*sizeof(uint32_t), &bytes_read, portMAX_DELAY);
		}
		else{
			memset(new_samples_raw, 0, sizeof(uint32_t) * CHUNK_SIZE * 2);
		}

		// Clip the sample value if it's too large, cast to floats
		for (uint16_t i = 0; i < CHUNK_SIZE*2; i+=4) {
			new_samples[i+0] = min(max((((int32_t)new_samples_raw[i+0]) >> 14) + 7000, (int32_t)-131072), (int32_t)131072) - 360;
			new_samples[i+1] = min(max((((int32_t)new_samples_raw[i+1]) >> 14) + 7000, (int32_t)-131072), (int32_t)131072) - 360;
			new_samples[i+2] = min(max((((int32_t)new_samples_raw[i+2]) >> 14) + 7000, (int32_t)-131072), (int32_t)131072) - 360;
			new_samples[i+3] = min(max((((int32_t)new_samples_raw[i+3]) >> 14) + 7000, (int32_t)-131072), (int32_t)131072) - 360;
		}

		// Convert audio from "18-bit" float range to -1.0 to 1.0 range
		dsps_mulc_f32(new_samples, new_samples, CHUNK_SIZE*2, recip_scale, 1, 1);

		// Add new chunk to audio history
		waveform_locked = true;
		shift_and_copy_arrays(sample_history, SAMPLE_HISTORY_LENGTH, new_samples, CHUNK_SIZE*2);
		
		// Used to sync GPU to this when needed
		waveform_locked = false;
		waveform_sync_flag = true;
	}, __func__);
}
