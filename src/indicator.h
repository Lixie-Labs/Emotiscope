#include "driver/ledc.h"

#define INDICATOR_LIGHT_GPIO (11)
#define INDICATOR_RESTING_BRIGHTNESS (0.25);
#define INDICATOR_MIN_BRIGHTNESS (0.005)
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

extern int16_t connection_status;
extern float standby_brightness;
extern float standby_breath;

bool status_blink_state = false;
uint32_t last_status_blink = 0;

uint32_t last_indicator_pulse_ms = 0;

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
        .hpoint         = 0,
		.flags = {
			.output_invert = 0,
		},
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void run_indicator_light(){
	extern self_test_steps_t self_test_step;
	
	profile_function([&]() {

		if(self_test_step == SELF_TEST_INACTIVE){
			if (connection_status == WL_CONNECTED) {
				indicator_brightness_target = INDICATOR_RESTING_BRIGHTNESS;

				if((device_touch_active == true) || (t_now_ms - last_indicator_pulse_ms <= 100)){
					indicator_brightness_target = 1.0;
				}
			}
			else{
				if(t_now_ms - last_status_blink >= STATUS_BLINK_INTERVAL_MS){
					status_blink_state = !status_blink_state;
					indicator_brightness_target = (float)status_blink_state;
					indicator_brightness = (float)status_blink_state;
					last_status_blink = t_now_ms;
				}
			}
		}
		else{ // Full brightness during test
			indicator_brightness_target = 1.0;
		}

		indicator_brightness = (indicator_brightness_target * 0.075) + indicator_brightness * 0.925;

		float output_brightness = clip_float(indicator_brightness*indicator_brightness+standby_breath)*standby_brightness*standby_brightness;
		output_brightness = output_brightness * (1.0-INDICATOR_MIN_BRIGHTNESS) + INDICATOR_MIN_BRIGHTNESS;

		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_MAX_DUTY * output_brightness);
		ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
	}, __func__);
}

void show_indicator(){
	last_indicator_pulse_ms = t_now_ms;
}