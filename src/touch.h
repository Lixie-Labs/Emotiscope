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
extern int16_t increment_mode();
extern uint8_t hold_blinks_queued;
extern bool hold_blink_state;

void init_touch(){
	touch_pins[TOUCH_LEFT].pin   = TOUCH_LEFT_PIN;
	touch_pins[TOUCH_CENTER].pin = TOUCH_CENTER_PIN;
	touch_pins[TOUCH_RIGHT].pin  = TOUCH_RIGHT_PIN;

	touch_pad_init();

	for(uint8_t i = 0; i < 3; i++){
		touch_pad_config((touch_pad_t)touch_pins[i].pin);

		touch_pins[i].touch_start = 0;
		touch_pins[i].touch_end = 0;
		touch_pins[i].touch_active = false;
		touch_pins[i].hold_active = false;
		touch_pins[i].touch_value = 0.00f;
		touch_pins[i].ambient_threshold = 10000000.0;
		touch_pins[i].touch_threshold   = 10000000.0;
		memset(touch_pins[i].touch_history, 0, sizeof(touch_pins[i].touch_history)); // Zero out the touch history
	}

	touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
}

void read_touch(){
	static uint32_t last_touch_read_time = 0;
	static uint8_t touch_history_index = 0;
	static bool filling_touch_history = true;

	static float touch_readings[3] = { 0.0, 0.0, 0.0 };

	uint32_t raw_touch_value;		
	for(uint8_t t = 0; t < 3; t++){
		touch_pad_read_raw_data((touch_pad_t)touch_pins[t].pin, &raw_touch_value);
		touch_readings[t] = touch_readings[t] * 0.9 + raw_touch_value * 0.1; // Smooth the input
	}

	if(t_now_ms < 5000){
		for(uint8_t t = 0; t < 3; t++){
			for(uint8_t i = 0; i < 50; i++){
				touch_pins[t].touch_history[i] = touch_readings[t];
			}
		}
	}

	if (t_now_ms - last_touch_read_time >= 100) {
		for(uint8_t t = 0; t < 3; t++){
			if(touch_pins[t].touch_active == false){
				touch_pins[t].touch_history[touch_history_index] = touch_readings[t];
			}
		}

		last_touch_read_time = t_now_ms;
		touch_history_index++;

		if(touch_history_index >= 50){
			touch_history_index = 0;
			if(filling_touch_history == true){
				filling_touch_history = false;
			}
		}
	}

	for(uint8_t t = 0; t < 3; t++){
		float touch_sum = 0.0;
		for(uint8_t i = 0; i < 50; i++){
			touch_sum += touch_pins[t].touch_history[i];
		}

		float touch_average = touch_sum / 50.0;

		touch_pins[t].ambient_threshold = touch_average * 1.0025;
		touch_pins[t].touch_threshold   = touch_average * 1.0200;

		if(touch_readings[t] >= touch_pins[t].ambient_threshold){
			if(touch_readings[t] < touch_pins[t].touch_threshold){
				if(filling_touch_history == false){
					touch_pins[t].touch_value = (touch_readings[t] - touch_pins[t].ambient_threshold) / (touch_pins[t].touch_threshold - touch_pins[t].ambient_threshold);
				}
			}
			else{
				if(filling_touch_history == false){
					touch_pins[t].touch_value = 1.0;
				}
			}
		}
		else{
			touch_pins[t].touch_value = 0.0;
		}
	}

	// Process touches -------------------------------------

	if(filling_touch_history == false){
		for(uint8_t t = 0; t < 3; t++){
			if(touch_pins[t].touch_value == 1.0){
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
								broadcast("reload_config");
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
	else if(touch_pins[TOUCH_LEFT].touch_active == true && touch_pins[TOUCH_RIGHT].touch_active == true){ // both hands held
		//printf("BOTH HANDS HELD\n");
		/*
		configuration.mirror_mode = !configuration.mirror_mode;

		touch_pins[TOUCH_LEFT].hold_active = true;
		touch_pins[TOUCH_RIGHT].hold_active = true;
		touch_pins[TOUCH_LEFT].touch_active = false;
		touch_pins[TOUCH_RIGHT].touch_active = false;
		*/
	}
}

void render_touches(){
	profile_function([&]() {
		if(light_modes[configuration.current_mode].type == LIGHT_MODE_TYPE_SYSTEM){ return; }

		if(touch_pins[TOUCH_LEFT].touch_value > 0.001){
			float glow_hue = 0.870;
			if(touch_pins[TOUCH_LEFT].touch_value >= 1.0){
				glow_hue = 0.060;
			}

			for(uint32_t i = 0; i < (NUM_LEDS>>2); i++){
				float progress = (float)i / ((NUM_LEDS>>2)-1);
				float brightness = (1.0-progress) * (sqrt(touch_pins[TOUCH_LEFT].touch_value));
				CRGBF glow_col = hsv(glow_hue, 1.0, brightness*0.5);

				leds[i] = add(leds[i], glow_col);
			}
		}

		if(touch_pins[TOUCH_RIGHT].touch_value > 0.001){
			float glow_hue = 0.870;
			if(touch_pins[TOUCH_RIGHT].touch_value >= 1.0){
				glow_hue = 0.060;
			}

			for(uint32_t i = 0; i < (NUM_LEDS>>2); i++){
				float progress = (float)i / ((NUM_LEDS>>2)-1);
				float brightness = (1.0-progress) * (sqrt(touch_pins[TOUCH_RIGHT].touch_value));
				CRGBF glow_col = hsv(glow_hue, 1.0, brightness*0.5);

				leds[(NUM_LEDS-1)-i] = add(leds[(NUM_LEDS-1)-i], glow_col);
			}
		}


		if(touch_pins[TOUCH_CENTER].touch_value > 0.001){
			float glow_hue = 0.870;
			if(touch_pins[TOUCH_CENTER].touch_value >= 1.0){
				glow_hue = 0.060;
			}

			for(int i = (NUM_LEDS>>2); i < ((NUM_LEDS>>2)*3); i++){
				float progress = 1.0 - (abs(i - (NUM_LEDS>>1)) / float(NUM_LEDS>>2));
				float brightness = (progress) * (sqrt(touch_pins[TOUCH_CENTER].touch_value));
				CRGBF glow_col = hsv(glow_hue, 1.0, brightness*0.5);

				leds[i] = add(leds[i], glow_col);
			}
		}
	}, __func__);
}