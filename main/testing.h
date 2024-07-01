#define RUN_COUNT 1000

void run_performance_test(){
	ESP_LOGI(TAG, "-- PERFORMANCE --------------------------------------\n");
	int64_t us_start = 0;
	int64_t us_end = 0;
	float us_taken = 0;


	// hsv performance test --------------------------------
	us_start = esp_timer_get_time();
	volatile CRGBF color = {0, 0, 0};
	for(int i = 0; i < RUN_COUNT; i++){

		// Test
		color = hsv(0.5, 1.0, 1.0);

	}
	us_end = esp_timer_get_time();
	us_taken = (us_end - us_start) / (float)RUN_COUNT;

	ESP_LOGI(TAG, "                HSV: %.3f us", us_taken);


	// apply_background performance test -------------------
	us_start = esp_timer_get_time();
	for(int i = 0; i < RUN_COUNT; i++){

		apply_background(0.01);	

	}
	us_end = esp_timer_get_time();
	us_taken = (us_end - us_start) / (float)RUN_COUNT;

	ESP_LOGI(TAG, "   apply_background: %.3f us", us_taken);


	// get_color_range_hue performance test ----------------
	us_start = esp_timer_get_time();
	for(int i = 0; i < RUN_COUNT; i++){

		get_color_range_hue(0.01);

	}
	us_end = esp_timer_get_time();
	us_taken = (us_end - us_start) / (float)RUN_COUNT;

	ESP_LOGI(TAG, "get_color_range_hue: %.3f us", us_taken);

}





