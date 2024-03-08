#include "driver/ledc.h"

#define INDICATOR_LIGHT_GPIO (11)
#define INDICATOR_RESTING_BRIGHTNESS (0.2);

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (INDICATOR_LIGHT_GPIO)
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

#define LEDC_MAX_DUTY           (8191)

float indicator_brightness = 0.0;
float indicator_brightness_target = 0.0;

extern volatile bool web_server_ready;
extern int16_t connection_status;

void init_indicator_light(){
	// Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	// Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
		.gpio_num       = LEDC_OUTPUT_IO,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
		.intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,        
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void run_indicator_light(){
	static uint32_t last_blink = -1000;
	static bool blink_state = 0;

	if(app_touch_active == true){
		indicator_brightness_target = 1.0;
	}
	else{
		if (connection_status == WL_CONNECTED) {
			indicator_brightness_target = INDICATOR_RESTING_BRIGHTNESS;
		}
		else{
			if(t_now_ms - last_blink >= 250){
				blink_state = !blink_state;

				indicator_brightness_target = blink_state;
				indicator_brightness = blink_state;

				last_blink = t_now_ms;
			}
		}
	}

	indicator_brightness = (indicator_brightness_target * 0.075) + indicator_brightness * 0.925;

	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_MAX_DUTY * (indicator_brightness*indicator_brightness));
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}