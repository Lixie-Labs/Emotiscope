#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <esp_check.h>
#include <esp_log.h>

rmt_channel_handle_t tx_chan = NULL;
rmt_encoder_handle_t led_encoder = NULL;

const uint32_t NUM_LEDS_TOTAL = 128;
const uint32_t NUM_LED_BITS = NUM_LEDS_TOTAL * 24;
const uint32_t NUM_LED_BYTES = NUM_LEDS_TOTAL * 3;
const uint32_t NUM_LEDS_PER_RMT = NUM_LEDS_TOTAL / 2;
const uint32_t NUM_BITS_PER_RMT = NUM_LED_BITS / 2;

ERGB8 leds_rmt[NUM_LEDS_TOTAL];

rmt_transmit_config_t tx_config = {
	.loop_count = 0,  // no transfer loop
	.flags = { .eot_level = 0, .queue_nonblocking = 0 }
};

// construct an encoder by combining primitive encoders
typedef struct {
    rmt_encoder_t base;           // the base "class" declares the standard encoder interface
    rmt_encoder_t *bytes_encoder; // use the bytes_encoder to encode the address and command data
    rmt_encode_state_t state; // record the current encoding state, i.e., we are in which encoding phase
} rmt_led_strip_encoder_t; 

static size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state){
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    switch (led_encoder->state) {
    case 0: // send RGB data
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = (rmt_encode_state_t)1; // switch to next state when current encoding session finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    // fall-through
    case 1: // send reset code
        // NOTHING? 
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
            state = RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

void init_rmt_driver() {
	rmt_tx_channel_config_t tx_chan_config = {
		.gpio_num = (gpio_num_t)13,		 // GPIO number
		.clk_src = RMT_CLK_SRC_DEFAULT,	 // select source clock
		.resolution_hz = 10000000,		 // 10 MHz tick resolution, i.e., 1 tick = 0.1 µs
		.mem_block_symbols = 1024,		 // memory block size, 64 * 4 = 256 Bytes
		.trans_queue_depth = 4,			 // set the number of transactions that can be pending in the background
		.flags = { .with_dma = 0 }
	};
	ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_chan));

	rmt_symbol_word_t bit_0 = { 4, 1, 6, 0 };
	rmt_symbol_word_t bit_1 = { 7, 1, 6, 0 };

	const rmt_bytes_encoder_config_t encoder_config = {
		.bit0 = bit_0,
		.bit1 = bit_1
	};

	rmt_new_bytes_encoder(&encoder_config, &led_encoder);
	
	ESP_ERROR_CHECK(rmt_enable(tx_chan));
}

void transmit_leds() {
	// ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_chan, portMAX_DELAY));
	rmt_transmit(tx_chan, led_encoder, (uint8_t *)leds_rmt, sizeof(leds_rmt), &tx_config);
}