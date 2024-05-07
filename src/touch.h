#include "driver/touch_pad.h"

#define TOUCH_LEFT_PIN   4
                     //  5  -----+-- mind the gap, maybe I'm paranoid of cross-talk
#define TOUCH_CENTER_PIN 6  //   |
                     //  7  -----+
#define TOUCH_RIGHT_PIN  8

#define TOUCH_HOLD_MS 500 // Less than this is a tap, more is a hold

typedef enum touch_position{
	TOUCH_LEFT = 0,
	TOUCH_CENTER = 1,
	TOUCH_RIGHT = 2
} touch_position;

touch_pin touch_pins[3];

volatile bool app_touch_active = false;
volatile bool slider_touch_active = false;
volatile bool device_touch_active = false;

extern void toggle_standby();
extern void increment_mode();
extern uint8_t hold_blinks_queued;
extern bool hold_blink_state;

void init_touch(){
	touch_pins[TOUCH_LEFT].pin   = TOUCH_LEFT_PIN;
	touch_pins[TOUCH_CENTER].pin = TOUCH_CENTER_PIN;
	touch_pins[TOUCH_RIGHT].pin  = TOUCH_RIGHT_PIN;

	touch_pins[TOUCH_LEFT].ambient   = configuration.ambient_left_threshold;
	touch_pins[TOUCH_CENTER].ambient = configuration.ambient_center_threshold;
	touch_pins[TOUCH_RIGHT].ambient  = configuration.ambient_right_threshold;

	touch_pins[TOUCH_LEFT].threshold   = configuration.touch_left_threshold;
	touch_pins[TOUCH_CENTER].threshold = configuration.touch_center_threshold;
	touch_pins[TOUCH_RIGHT].threshold  = configuration.touch_right_threshold;

	touch_pad_init();

	for(uint8_t i = 0; i < 3; i++){
		touch_pad_config((touch_pad_t)touch_pins[i].pin);
	}

	/*
	touch_pad_set_measurement_interval(TOUCH_PAD_SLEEP_CYCLE_DEFAULT);
    touch_pad_set_charge_discharge_times(TOUCH_PAD_MEASURE_CYCLE_DEFAULT);
    touch_pad_set_voltage(TOUCH_PAD_HIGH_VOLTAGE_THRESHOLD, TOUCH_PAD_LOW_VOLTAGE_THRESHOLD, TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD);
    touch_pad_set_idle_channel_connect(TOUCH_PAD_IDLE_CH_CONNECT_DEFAULT);
    
	touch_pad_set_cnt_mode((touch_pad_t)TOUCH_CENTER, TOUCH_PAD_SLOPE_DEFAULT, TOUCH_PAD_TIE_OPT_DEFAULT);
	*/

	touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
}

void study_pin(){
	pinMode(TOUCH_CENTER_PIN, INPUT_PULLUP);
	pinMode(TOUCH_LEFT_PIN, INPUT_PULLUP);
	pinMode(TOUCH_RIGHT_PIN, INPUT_PULLUP);

	printf("TOUCH CENTER: %d | TOUCH LEFT: %d | TOUCH RIGHT: %d\n", digitalRead(TOUCH_CENTER_PIN), digitalRead(TOUCH_LEFT_PIN), digitalRead(TOUCH_RIGHT_PIN));
}

void read_touch(){
	static uint8_t iter = 0;

	iter++;
	if(iter >= 3){
		iter = 0;

		for(uint8_t t = 0; t < 3; t++){
			uint32_t raw_touch_value;		
			touch_pad_read_raw_data((touch_pad_t)touch_pins[t].pin, &raw_touch_value);

			touch_pins[t].touch_value = raw_touch_value * 0.9 + touch_pins[t].touch_value * 0.1; // Smooth the input

			if(touch_pins[t].touch_value >= touch_pins[t].threshold){
				if(touch_pins[t].touch_active == false){
					touch_pins[t].touch_active = true;
					touch_pins[t].hold_active = false;
					touch_pins[t].touch_start = t_now_ms;
				}
				else{
					uint32_t touch_hold_time = t_now_ms - touch_pins[t].touch_start;

					if(touch_hold_time >= TOUCH_HOLD_MS){
						// Handle hold
						if(touch_pins[t].hold_active == false){
							touch_pins[t].hold_active = true;
							printf("HOLD TOUCH TRIGGER ON PIN: %d\n", touch_pins[t].pin);

							if(touch_pins[t].pin == TOUCH_LEFT_PIN){
								// nothing
							}
							else if(touch_pins[t].pin == TOUCH_CENTER_PIN){
								toggle_standby();
							}
							else if(touch_pins[t].pin == TOUCH_RIGHT_PIN){
								// nothing
							}
						}
					}
				}
			}
			else{
				if(touch_pins[t].touch_active == true){
					touch_pins[t].touch_active = false;
					touch_pins[t].hold_active = false;
					touch_pins[t].touch_end = t_now_ms;
					int32_t touch_duration = touch_pins[t].touch_end - touch_pins[t].touch_start;

					if(touch_duration < TOUCH_HOLD_MS){
						// Handle tap
						printf("TAP TOUCH TRIGGER CENTER ON PIN: %d\n", touch_pins[t].pin);

						if(touch_pins[t].pin == TOUCH_LEFT_PIN){
							// nothing
						}
						else if(touch_pins[t].pin == TOUCH_CENTER_PIN){
							if(EMOTISCOPE_ACTIVE == true){
								increment_mode();
								save_config_delayed();
							}
							else{
								toggle_standby();
							}
						}
						else if(touch_pins[t].pin == TOUCH_RIGHT_PIN){
							// nothing
						}
					}
				}
			}
		}

		// Print touch values (floats with 3 decimal points)
		//printf("TOUCH LEFT: %.3f | TOUCH CENTER: %.3f | TOUCH RIGHT: %.3f\n", touch_pins[0].touch_value, touch_pins[1].touch_value, touch_pins[2].touch_value);
	}

	if(touch_pins[TOUCH_LEFT].touch_active == true || touch_pins[TOUCH_CENTER].touch_active == true || touch_pins[TOUCH_RIGHT].touch_active == true){
		device_touch_active = true;
	}
	else{
		device_touch_active = false;
	}

	if(touch_pins[TOUCH_LEFT].hold_active == true && touch_pins[TOUCH_RIGHT].touch_active == false){ // Left hold active
		configuration.color -= 0.0015;
		if(configuration.color < 0.0){
			configuration.color += 1.0;
		}

		save_config_delayed();
	}
	else if(touch_pins[TOUCH_RIGHT].hold_active == true && touch_pins[TOUCH_LEFT].touch_active == false){ // Right hold active
		configuration.color += 0.0015;
		if(configuration.color > 1.0){
			configuration.color -= 1.0;
		}

		save_config_delayed();
	}
	else if(touch_pins[TOUCH_LEFT].hold_active == true && touch_pins[TOUCH_RIGHT].hold_active == true){ // both hands held
		printf("BOTH HANDS HELD\n");
		// TODO: Add noise calibration visual countdown and trigger when both "ears" covered by hands
	}
}

void render_touches(){
	static float left_distance_smooth = 0.0;
	static float right_distance_smooth = 0.0;
	static float center_distance_smooth = 0.0;

	float left_touch_value = touch_pins[TOUCH_LEFT].touch_value;
	if(left_touch_value > touch_pins[TOUCH_LEFT].ambient){
		if(left_touch_value < touch_pins[TOUCH_LEFT].threshold){
			float distance = (left_touch_value - touch_pins[TOUCH_LEFT].ambient) / (touch_pins[TOUCH_LEFT].threshold - touch_pins[TOUCH_LEFT].ambient);
			left_distance_smooth = left_distance_smooth * 0.9 + distance * 0.1;
			for(uint32_t i = 0; i < 32; i++){
				float progress = (float)i / 31.0;
				float brightness = (1.0-progress) * (sqrt(sqrt(left_distance_smooth)));
				CRGBF glow_col = hsv(0.870, 1.0, brightness*brightness*0.5);

				leds[i] = add(leds[i], glow_col);
			}
		}
		else{
			float distance = 1.0;
			left_distance_smooth = left_distance_smooth * 0.9 + distance * 0.1;
			for(uint32_t i = 0; i < 32; i++){
				float progress = (float)i / 31.0;
				float brightness = (1.0-progress);
				CRGBF glow_col = hsv(0.060, 0.9, brightness*brightness*0.75);

				leds[i] = add(leds[i], glow_col);
			}
		}
	}
	else{
		float distance = 0.0;
		left_distance_smooth = left_distance_smooth * 0.9 + distance * 0.1;
		if(left_distance_smooth > 0.005){
			for(uint32_t i = 0; i < 32; i++){
				float progress = (float)i / 31.0;
				float brightness = (1.0-progress) * (sqrt(sqrt(left_distance_smooth)));
				CRGBF glow_col = hsv(0.870, 1.0, brightness*brightness*0.5);

				leds[i] = add(leds[i], glow_col);
			}
		}
	}

	float right_touch_value = touch_pins[TOUCH_RIGHT].touch_value;
	if(right_touch_value > touch_pins[TOUCH_RIGHT].ambient){
		if(right_touch_value < touch_pins[TOUCH_RIGHT].threshold){
			float distance = (right_touch_value - touch_pins[TOUCH_RIGHT].ambient) / (touch_pins[TOUCH_RIGHT].threshold - touch_pins[TOUCH_RIGHT].ambient);
			right_distance_smooth = right_distance_smooth * 0.9 + distance * 0.1;
			for(uint32_t i = 0; i < 32; i++){
				float progress = (float)i / 31.0;
				float brightness = (1.0-progress) * (sqrt(sqrt(right_distance_smooth)));
				CRGBF glow_col = hsv(0.870, 1.0, brightness*brightness*0.5);

				leds[(NUM_LEDS-1)-i] = add(leds[(NUM_LEDS-1)-i], glow_col);
			}
		}
		else{
			float distance = 1.0;
			right_distance_smooth = right_distance_smooth * 0.9 + distance * 0.1;
			for(uint32_t i = 0; i < 32; i++){
				float progress = (float)i / 31.0;
				float brightness = (1.0-progress);
				CRGBF glow_col = hsv(0.060, 0.9, brightness*brightness*0.75);

				leds[(NUM_LEDS-1)-i] = add(leds[(NUM_LEDS-1)-i], glow_col);
			}
		}
	}
	else{
		float distance = 0.0;
		right_distance_smooth = right_distance_smooth * 0.9 + distance * 0.1;
		if(right_distance_smooth > 0.005){
			for(uint32_t i = 0; i < 32; i++){
				float progress = (float)i / 31.0;
				float brightness = (1.0-progress) * (sqrt(sqrt(right_distance_smooth)));
				CRGBF glow_col = hsv(0.870, 1.0, brightness*brightness*0.5);

				leds[(NUM_LEDS-1)-i] = add(leds[(NUM_LEDS-1)-i], glow_col);
			}
		}
	}

	// Center touch is drawn using the center 64 LEDs
	float center_touch_value = touch_pins[TOUCH_CENTER].touch_value;
	if(center_touch_value > touch_pins[TOUCH_CENTER].ambient){
		if(center_touch_value < touch_pins[TOUCH_CENTER].threshold){
			float distance = (center_touch_value - touch_pins[TOUCH_CENTER].ambient) / (touch_pins[TOUCH_CENTER].threshold - touch_pins[TOUCH_CENTER].ambient);
			center_distance_smooth = center_distance_smooth * 0.9 + distance * 0.1;
			for(int i = 32; i < 96; i++){
				float progress = 1.0 - (abs(i - 64) / 32.0); // Adjusted progress calculation
				float brightness = (progress) * (sqrt(sqrt(center_distance_smooth)));
				CRGBF glow_col = hsv(0.870, 1.0, brightness*brightness*0.5);

				leds[i] = add(leds[i], glow_col);
			}
		}
		else{
			float distance = 1.0;
			center_distance_smooth = center_distance_smooth * 0.9 + distance * 0.1;
			for(int i = 32; i < 96; i++){
				float progress = 1.0 - (abs(i - 64) / 32.0); // Adjusted progress calculation
				float brightness = (progress);
				CRGBF glow_col = hsv(0.060, 0.9, brightness*brightness*0.75);
	
				leds[i] = add(leds[i], glow_col);
			}
		}
	}
	else{
		float distance = 0.0;
		center_distance_smooth = center_distance_smooth * 0.9 + distance * 0.1;
		if(center_distance_smooth > 0.005){
			for(int i = 32; i < 96; i++){
				float progress = 1.0 - (abs(i - 64) / 32.0); // Adjusted progress calculation
				float brightness = (progress) * (sqrt(sqrt(center_distance_smooth)));
				CRGBF glow_col = hsv(0.870, 1.0, brightness*brightness*0.5);

				leds[i] = add(leds[i], glow_col);
			}
		}
	}

}