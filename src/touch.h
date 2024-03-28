#include "driver/touch_pad.h"

#define TOUCH_LEFT_PIN   4
                     //  5  -----+-- mind the gap, maybe I'm paranoid of cross-talk
#define TOUCH_CENTER_PIN 6  //   |
                     //  7  -----+
#define TOUCH_RIGHT_PIN  8

#define TOUCH_HOLD_MS 500 // Less than this is a tap, more is a hold

touch_pin touch_pins[3];

volatile bool app_touch_active = false;
volatile bool slider_touch_active = false;
volatile bool touch_active = false;

extern void toggle_standby();
extern void increment_mode();
extern uint8_t hold_blinks_queued;
extern bool hold_blink_state;

void init_touch(){
	touch_pins[0].pin = TOUCH_LEFT_PIN;
	touch_pins[1].pin = TOUCH_CENTER_PIN;
	touch_pins[2].pin = TOUCH_RIGHT_PIN;

	touch_pins[0].threshold = 33000;
	touch_pins[1].threshold = 95000;
	touch_pins[2].threshold = 64000;

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
	static uint8_t current_pin = 0;
	static uint8_t iter = 0;

	iter++;
	if(iter >= 3){
		iter = 0;

		current_pin++;

		uint8_t t = current_pin % 3;

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

		// Print touch values (floats with 3 decimal points)
		//printf("TOUCH LEFT: %.3f | TOUCH CENTER: %.3f | TOUCH RIGHT: %.3f\n", touch_pins[0].touch_value, touch_pins[1].touch_value, touch_pins[2].touch_value);
	}

	if(touch_pins[0].touch_active == true || touch_pins[1].touch_active == true || touch_pins[2].touch_active == true){
		touch_active = true;
	}
	else{
		touch_active = false;
	}

	if(touch_pins[0].hold_active == true){ // Left hold active
		configuration.hue -= 0.001;
		if(configuration.hue < 0.0){
			configuration.hue += 1.0;
		}

		save_config_delayed();
	}

	else if(touch_pins[2].hold_active == true){ // Right hold active
		configuration.hue += 0.001;
		if(configuration.hue > 1.0){
			configuration.hue -= 1.0;
		}

		save_config_delayed();
	}
}