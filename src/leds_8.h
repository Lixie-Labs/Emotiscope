#include "driver/rmt.h"
#define RMT_CLK_DIV 8 // 80MHz --> 10MHz
#define RMT_TICK  (80000000/RMT_CLK_DIV/10000000) // 1 tick = 100ns

gpio_num_t RMT_A_GPIO = (gpio_num_t)13;
gpio_num_t RMT_B_GPIO = (gpio_num_t)12;

const uint32_t NUM_LEDS_TOTAL = 128;
const uint32_t NUM_LED_BITS  = NUM_LEDS_TOTAL*24;
const uint32_t NUM_LED_BYTES = NUM_LEDS_TOTAL*3;
const uint32_t NUM_LEDS_PER_RMT  = NUM_LEDS_TOTAL / 2;
const uint32_t NUM_BITS_PER_RMT  = NUM_LED_BITS / 2;

rmt_item32_t rmt_items_a[NUM_BITS_PER_RMT * sizeof(rmt_item32_t)];
rmt_item32_t rmt_items_b[NUM_BITS_PER_RMT * sizeof(rmt_item32_t)];

struct ERGB8 {
	uint8_t g;
	uint8_t r;
	uint8_t b;
};

ERGB8 leds_rmt[NUM_LEDS_TOTAL];

// Initialize RMT for transmission
void rmt_tx_init(rmt_channel_t channel, gpio_num_t gpio_num) {
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = channel,
        .gpio_num = gpio_num,
        .clk_div = RMT_CLK_DIV, 
        .mem_block_num = 1, // Number of memory blocks
        .tx_config = {
		    .carrier_freq_hz = 10000000,
			.carrier_level = RMT_CARRIER_LEVEL_HIGH,
			.idle_level = RMT_IDLE_LEVEL_LOW,
            .carrier_duty_percent = 50,
			.loop_count = 0,
			.carrier_en = false,
			.loop_en = false,
			.idle_output_en = true,
        },
    };

    // Configure the RMT peripheral based on the config
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);
}

// Function to prepare RMT items from a bit array
void prepare_rmt_items(rmt_item32_t* items, uint16_t offset) {
	uint8_t* bit_array = (uint8_t*)leds_rmt;

	uint8_t bit_index = 0;
	uint16_t byte_index = offset*3;

    for (size_t i = 0; i < NUM_BITS_PER_RMT; i++) {
		uint8_t bit_val = bitRead(bit_array[byte_index], 7-bit_index);

        if (bit_val == 0) {
            // Zero bit: HIGH for 3 ticks, LOW for 6 ticks
            items[i].level0 = 1;
            items[i].duration0 = RMT_TICK * 4;
            items[i].level1 = 0;
            items[i].duration1 = RMT_TICK * 6;
        } else {
            // One bit: HIGH for 7 ticks, LOW for 6 ticks
            items[i].level0 = 1;
            items[i].duration0 = RMT_TICK * 7;
            items[i].level1 = 0;
            items[i].duration1 = RMT_TICK * 6;
        }

		bit_index++;
		if(bit_index >= 8){
			bit_index = 0;
			byte_index++;
		}
    }
}

// Function to transmit the bit array
void transmit_leds() {
	rmt_wait_tx_done(RMT_CHANNEL_0, portMAX_DELAY);
	rmt_wait_tx_done(RMT_CHANNEL_1, portMAX_DELAY);

    prepare_rmt_items(rmt_items_a, 0);
	prepare_rmt_items(rmt_items_b, 64);

	rmt_write_items(RMT_CHANNEL_0, rmt_items_a, NUM_LED_BITS, false);
	rmt_write_items(RMT_CHANNEL_1, rmt_items_b, NUM_LED_BITS, false);
}