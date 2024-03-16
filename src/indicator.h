#include "driver/ledc.h"

#define INDICATOR_LIGHT_GPIO (11)
#define INDICATOR_RESTING_BRIGHTNESS (0.25);
#define INDICATOR_MIN_BRIGHTNESS (0.01)
#define STATUS_BLINK_INTERVAL_MS (400)
#define HOLD_BLINK_INTERVAL_MS (200)

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
extern bool touch_active;
extern float standby_brightness;
extern float standby_breath;

bool status_blink_state = false;
uint32_t last_status_blink = 0;

uint8_t hold_blinks_queued = 0;
bool hold_blink_state = false;
uint32_t last_hold_blink = 0;

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

			// if blinks == 0, toggle the indicator target on and off every blink_interval_ms. Only modify the indicator duty cycle if blink_state == true
			if(touch_active == true){
				if(hold_blinks_queued > 0){
					if(t_now_ms - last_hold_blink >= HOLD_BLINK_INTERVAL_MS){
						hold_blink_state = !hold_blink_state;

						if(hold_blink_state == true){
							indicator_brightness_target = 1.0;
							indicator_brightness = 1.0;
						}
						else{
							indicator_brightness_target = 0.0;
							indicator_brightness = 0.0;
							hold_blinks_queued--;
						}

						last_hold_blink = t_now_ms;					}
				}
				else{
					indicator_brightness_target = 1.0;
				}
			}
		}
		else{
			if(t_now_ms - last_status_blink >= STATUS_BLINK_INTERVAL_MS){
				status_blink_state = !status_blink_state;

				indicator_brightness_target = status_blink_state;
				indicator_brightness = status_blink_state;

				last_status_blink = t_now_ms;
			}
		}
	}

	indicator_brightness = (indicator_brightness_target * 0.075) + indicator_brightness * 0.925;

	float output_brightness = clip_float(indicator_brightness*indicator_brightness+standby_breath)*standby_brightness*standby_brightness;
	output_brightness = output_brightness * (1.0-INDICATOR_MIN_BRIGHTNESS) + INDICATOR_MIN_BRIGHTNESS;

	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_MAX_DUTY * output_brightness);
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}