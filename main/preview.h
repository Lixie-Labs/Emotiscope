uint8_t screen_preview[SCREEN_PREVIEW_SIZE*3];

void run_screen_preview() {
	start_profile(__COUNTER__, __func__);
	for(uint8_t i = 0; i < SCREEN_PREVIEW_SIZE; i++){
		CRGBF color_a = leds_last[i*2+0];
		CRGBF color_b = leds_last[i*2+1];

		screen_preview[i*3+0] = (uint8_t)(color_a.r * 255);
		screen_preview[i*3+1] = (uint8_t)(color_a.g * 255);
		screen_preview[i*3+2] = (uint8_t)(color_a.b * 255);
	}
	end_profile();
}