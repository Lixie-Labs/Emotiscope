#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <esp_check.h>
#include <esp_log.h>

rmt_channel_handle_t tx_chan_a = NULL;
rmt_channel_handle_t tx_chan_b = NULL;
rmt_encoder_handle_t led_encoder_a = NULL;
rmt_encoder_handle_t led_encoder_b = NULL;

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} rmt_led_strip_encoder_t;

rmt_led_strip_encoder_t strip_encoder_a;
rmt_led_strip_encoder_t strip_encoder_b;

const uint32_t NUM_LEDS_TOTAL = 128;

rmt_transmit_config_t tx_config = {
	.loop_count = 0,  // no transfer loop
	.flags = { .eot_level = 0, .queue_nonblocking = 0 }
};

typedef struct {
    uint32_t resolution; /*!< Encoder resolution, in Hz */
} led_strip_encoder_config_t;

static const char *TAG = "led_encoder";

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
		.gpio_num = (gpio_num_t)13,		 // GPIO number
		.clk_src = RMT_CLK_SRC_DEFAULT,	 // select source clock
		.resolution_hz = 10000000,		 // 10 MHz tick resolution, i.e., 1 tick = 0.1 µs
		.mem_block_symbols = 128,		 // memory block size, 64 * 4 = 256 Bytes
		.trans_queue_depth = 2,			 // set the number of transactions that can be pending in the background
		.flags = { .with_dma = 0 }
	};

	rmt_tx_channel_config_t tx_chan_b_config = {
		.gpio_num = (gpio_num_t)12,		 // GPIO number
		.clk_src = RMT_CLK_SRC_DEFAULT,	 // select source clock
		.resolution_hz = 10000000,		 // 10 MHz tick resolution, i.e., 1 tick = 0.1 µs
		.mem_block_symbols = 128,		 // memory block size, 64 * 4 = 256 Bytes
		.trans_queue_depth = 2,			 // set the number of transactions that can be pending in the background
		.flags = { .with_dma = 0 }
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
}

static uint8_t raw_led_data[NUM_LEDS_TOTAL*3];
extern CRGBF leds[NUM_LEDS_TOTAL];

void quantize_color() {
	const float dither_table[4] = {0.25, 0.50, 0.75, 1.00};
	static uint8_t dither_step = 0;
	dither_step++;

	float decimal_r; float decimal_g; float decimal_b;
	uint8_t whole_r; uint8_t whole_g; uint8_t whole_b;
	float   fract_r; float   fract_g; float   fract_b;

	for (uint16_t i = 0; i < NUM_LEDS_TOTAL; i++) {
		// RED #####################################################
		decimal_r = leds[i].r * 254;
		whole_r = decimal_r;
		fract_r = decimal_r - whole_r;
		raw_led_data[3*i+1] = whole_r + (fract_r >= dither_table[(dither_step + i) % 4]);
		
		// GREEN #####################################################
		decimal_g = leds[i].g * 254;
		whole_g = decimal_g;
		fract_g = decimal_g - whole_g;
		raw_led_data[3*i+0] = whole_g + (fract_g >= dither_table[(dither_step + i) % 4]);

		// BLUE #####################################################
		decimal_b = leds[i].b * 254;
		whole_b = decimal_b;
		fract_b = decimal_b - whole_b;
		raw_led_data[3*i+2] = whole_b + (fract_b >= dither_table[(dither_step + i) % 4]);
	}
}

void transmit_leds() {
	rmt_tx_wait_all_done(tx_chan_a, portMAX_DELAY);
	rmt_tx_wait_all_done(tx_chan_b, portMAX_DELAY);

	// Quantize the floating point color to 8-bit with dithering
	memset(raw_led_data, 0, NUM_LEDS_TOTAL*3);
	quantize_color();

	rmt_transmit(tx_chan_a, led_encoder_a, raw_led_data, (sizeof(raw_led_data) >> 1), &tx_config);
	rmt_transmit(tx_chan_b, led_encoder_b, raw_led_data+((NUM_LEDS_TOTAL>>1)*3), (sizeof(raw_led_data) >> 1), &tx_config);
}