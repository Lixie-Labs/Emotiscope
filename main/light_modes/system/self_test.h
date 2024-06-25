extern void enter_queued_light_mode();
self_test_steps_t self_test_step = SELF_TEST_INACTIVE;

void draw_self_test(){
	static uint32_t test_start_time = 0;

	if(self_test_step == SELF_TEST_STEP_START){
		test_start_time = t_now_ms;
		self_test_step = SELF_TEST_STEP_LED;
		return;
	}
	else if(self_test_step == SELF_TEST_STEP_LED){
		if(t_now_ms - test_start_time < 1000){
			fill_color( leds, NUM_LEDS, (CRGBF){0.0, 0.0, 0.0} );
		}
		else if(t_now_ms - test_start_time < 2000){
			fill_color( leds, NUM_LEDS, (CRGBF){0.5, 0.0, 0.0} );
		}
		else if(t_now_ms - test_start_time < 3000){
			fill_color( leds, NUM_LEDS, (CRGBF){0.0, 0.5, 0.0} );
		}
		else if(t_now_ms - test_start_time < 4000){
			fill_color( leds, NUM_LEDS, (CRGBF){0.0, 0.0, 0.5} );
		}
		else if(t_now_ms - test_start_time < 5000){
			fill_color( leds, NUM_LEDS, (CRGBF){0.25, 0.25, 0.25} );
		}
		else{
			fill_color( leds, NUM_LEDS, (CRGBF){0.0, 0.0, 0.0} );
			self_test_step = SELF_TEST_STEP_COMPLETE;
		}
	}
	else if(self_test_step == SELF_TEST_STEP_COMPLETE){
		self_test_step = SELF_TEST_INACTIVE;
		enter_queued_light_mode();
	}
}