// Define the touch pad pin number, replace TOUCH_PIN with the actual touch pad you're using
#define TOUCH_PIN 1

// Threshold for considering a touch detected
#define TOUCH_THRESHOLD 40000

#define TOUCH_HOLD_MS 500

uint32_t touch_start = 0;
uint32_t touch_end   = 0;

bool touch_active = false;
bool hold_active = false;

extern void toggle_standby();
extern void increment_mode();

void read_touch( uint32_t t_now_ms ){
	static float touch_value = 0;

	int32_t raw_touch_value = touchRead( TOUCH_PIN );
	touch_value = raw_touch_value * 0.15 + touch_value * 0.85; // Smooth the input

	if(touch_value >= TOUCH_THRESHOLD){
		if(touch_active == false){
			touch_active = true;
			hold_active = false;
			touch_start = t_now_ms;
		}
		else{
			uint32_t touch_hold_time = t_now_ms - touch_start;

			if(touch_hold_time >= TOUCH_HOLD_MS){
				// Handle hold
				if(hold_active == false){
					hold_active = true;
					printf("HOLD TOUCH TRIGGER\n");
					toggle_standby();
				}
			}
		}
	}
	else{
		if(touch_active == true){
			touch_active = false;
			hold_active = false;
			touch_end = t_now_ms;
			int32_t touch_duration = touch_end-touch_start;

			if(touch_duration < TOUCH_HOLD_MS){
				// Handle tap
				printf("TAP TOUCH TRIGGER\n");
				increment_mode();
			}
		}
	}
}