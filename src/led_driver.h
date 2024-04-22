#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <esp_check.h>
#include <esp_log.h>

#define LED_DATA_1_PIN ( 21 )
#define LED_DATA_2_PIN ( 17 )

// It won't void any kind of stupid warranty, but things will *definitely* break at this point if you change this number.
#define NUM_LEDS ( 128 )

// 32-bit color input
extern CRGBF leds[NUM_LEDS];

CRGBF dither_error[NUM_LEDS];

// 8-bit color output
static uint8_t raw_led_data[NUM_LEDS*3];

rmt_channel_handle_t tx_chan_a = NULL;
rmt_channel_handle_t tx_chan_b = NULL;
rmt_encoder_handle_t led_encoder_a = NULL;
rmt_encoder_handle_t led_encoder_b = NULL;

uint32_t lfsr = 0xACE1u;  // Initial seed for LFSR
const uint32_t polynomial = 0x10000000u;  // Polynomial for LFSR

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} rmt_led_strip_encoder_t;

rmt_led_strip_encoder_t strip_encoder_a;
rmt_led_strip_encoder_t strip_encoder_b;

rmt_transmit_config_t tx_config = {
	.loop_count = 0,  // no transfer loop
	.flags = { .eot_level = 0, .queue_nonblocking = 0 }
};

typedef struct {
    uint32_t resolution; /*!< Encoder resolution, in Hz */
} led_strip_encoder_config_t;

static const char *TAG = "led_encoder";

void init_random_dither_error(){
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		dither_error[i].r = random(0, 255) / 255.0;
		dither_error[i].g = random(0, 255) / 255.0;
		dither_error[i].b = random(0, 255) / 255.0;
	}
}

IRAM_ATTR static size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state){
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    switch (led_encoder->state) {
    case 0: // send RGB data
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = 1; // switch to next state when current encoding session finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = (rmt_encode_state_t)(state | (uint32_t)RMT_ENCODING_MEM_FULL);
            goto out; // yield if there's no free space for encoding artifacts
        }
    // fall-through
    case 1: // send reset code
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
                                                sizeof(led_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
			state = (rmt_encode_state_t)(state | (uint32_t)RMT_ENCODING_COMPLETE);
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
			state = (rmt_encode_state_t)(state | (uint32_t)RMT_ENCODING_MEM_FULL);
            goto out; // yield if there's no free space for encoding artifacts
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

static esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder){
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder){
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder_a, rmt_encoder_handle_t *ret_encoder_b){
    esp_err_t ret = ESP_OK;

	strip_encoder_a.base.encode = rmt_encode_led_strip;
    strip_encoder_a.base.del    = rmt_del_led_strip_encoder;
    strip_encoder_a.base.reset  = rmt_led_strip_encoder_reset;

	strip_encoder_b.base.encode = rmt_encode_led_strip;
    strip_encoder_b.base.del    = rmt_del_led_strip_encoder;
    strip_encoder_b.base.reset  = rmt_led_strip_encoder_reset;

    // different led strip might have its own timing requirements, following parameter is for WS2812
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = { 4, 1, 6, 0 },
        .bit1 = { 7, 1, 6, 0 },
		.flags = { .msb_first = 1 }
    };
    
	rmt_new_bytes_encoder(&bytes_encoder_config, &strip_encoder_a.bytes_encoder);
	rmt_new_bytes_encoder(&bytes_encoder_config, &strip_encoder_b.bytes_encoder);
    rmt_copy_encoder_config_t copy_encoder_config = {};
    rmt_new_copy_encoder(&copy_encoder_config, &strip_encoder_a.copy_encoder);
	rmt_new_copy_encoder(&copy_encoder_config, &strip_encoder_b.copy_encoder);

    strip_encoder_a.reset_code = (rmt_symbol_word_t) { 250, 0, 250, 0 };
    strip_encoder_b.reset_code = (rmt_symbol_word_t) { 250, 0, 250, 0 };

    *ret_encoder_a = &strip_encoder_a.base;
    *ret_encoder_b = &strip_encoder_b.base;
    return ESP_OK;
}

void init_rmt_driver() {
	printf("init_rmt_driver\n");
	rmt_tx_channel_config_t tx_chan_a_config = {
		.gpio_num = (gpio_num_t)LED_DATA_1_PIN,	// GPIO number
		.clk_src = RMT_CLK_SRC_DEFAULT,	 // select source clock
		.resolution_hz = 10000000,		 // 10 MHz tick resolution, i.e., 1 tick = 0.1 µs
		.mem_block_symbols = 64,		 // memory block size, 64 * 4 = 256 Bytes
		.trans_queue_depth = 4,			 // set the number of transactions that can be pending in the background
		.intr_priority = 99,
		.flags = { .with_dma = 0 },
		//.flags = { .invert_out = 1, .with_dma = 0 }, // For level shifter
	};

	rmt_tx_channel_config_t tx_chan_b_config = {
		.gpio_num = (gpio_num_t)LED_DATA_2_PIN, // GPIO number
		.clk_src = RMT_CLK_SRC_DEFAULT,	 // select source clock
		.resolution_hz = 10000000,		 // 10 MHz tick resolution, i.e., 1 tick = 0.1 µs
		.mem_block_symbols = 64,		 // memory block size, 64 * 4 = 256 Bytes
		.trans_queue_depth = 4,			 // set the number of transactions that can be pending in the background
		.intr_priority = 99,
		.flags = { .with_dma = 0 },
		//.flags = { .invert_out = 1, .with_dma = 0 },
	};

	printf("rmt_new_tx_channel\n");
	ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_a_config, &tx_chan_a));
	ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_b_config, &tx_chan_b));

	ESP_LOGI(TAG, "Install led strip encoder");
    led_strip_encoder_config_t encoder_config = {
        .resolution = 10000000,
    };
	printf("rmt_new_led_strip_encoder\n");
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder_a, &led_encoder_b));
	
	printf("rmt_enable\n");
	ESP_ERROR_CHECK(rmt_enable(tx_chan_a));
	ESP_ERROR_CHECK(rmt_enable(tx_chan_b));

	init_random_dither_error();
}

void quantize_color_error(bool temporal_dithering){
	if(temporal_dithering == true){
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			float leds_r_scaled = leds[i].r * 255;
			float leds_g_scaled = leds[i].g * 255;
			float leds_b_scaled = leds[i].b * 255;
			
			raw_led_data[3*i+1] = (uint8_t)(leds_r_scaled);
			raw_led_data[3*i+0] = (uint8_t)(leds_g_scaled);
			raw_led_data[3*i+2] = (uint8_t)(leds_b_scaled);

			float new_error_r = leds_r_scaled - raw_led_data[3*i+1];
			float new_error_g = leds_g_scaled - raw_led_data[3*i+0];
			float new_error_b = leds_b_scaled - raw_led_data[3*i+2];

			const float dither_error_threshold = 0.055;
			if(new_error_r >= dither_error_threshold){ dither_error[i].r += new_error_r; }
			if(new_error_g >= dither_error_threshold){ dither_error[i].g += new_error_g; }
			if(new_error_b >= dither_error_threshold){ dither_error[i].b += new_error_b; }

			if(dither_error[i].r >= 1.0){ raw_led_data[3*i+1] += 1; dither_error[i].r -= 1.0; }
			if(dither_error[i].g >= 1.0){ raw_led_data[3*i+0] += 1; dither_error[i].g -= 1.0; }
			if(dither_error[i].b >= 1.0){ raw_led_data[3*i+2] += 1; dither_error[i].b -= 1.0; }
		}
	}
	else{
		for (uint16_t i = 0; i < NUM_LEDS; i++) {
			raw_led_data[3*i+1] = (uint8_t)(leds[i].r * 255);
			raw_led_data[3*i+0] = (uint8_t)(leds[i].g * 255);
			raw_led_data[3*i+2] = (uint8_t)(leds[i].b * 255);
		}
	}
}

IRAM_ATTR void transmit_leds() {
	// Wait here if previous frame transmission has not yet completed
	rmt_tx_wait_all_done(tx_chan_a, portMAX_DELAY);
	rmt_tx_wait_all_done(tx_chan_b, portMAX_DELAY);

	// Clear the 8-bit buffer	
	memset(raw_led_data, 0, NUM_LEDS*3);

	// Quantize the floating point color to 8-bit with dithering
	//
	// This allows the 8-bit LEDs to emulate the look of a higher bit-depth using persistence of vision tricks
	// The contents of the floating point CRGBF "leds" array are downsampled into the in alternating ways hundreds of
	// time 
	quantize_color_error(configuration.temporal_dithering);
	//add_noise();

	// Get to safety, THE PHOTONS ARE COMING!!!
	if(filesystem_ready == true){
		rmt_transmit(tx_chan_a, led_encoder_a, raw_led_data, (sizeof(raw_led_data) >> 1), &tx_config);
		rmt_transmit(tx_chan_b, led_encoder_b, raw_led_data+((NUM_LEDS>>1)*3), (sizeof(raw_led_data) >> 1), &tx_config);
	}
}