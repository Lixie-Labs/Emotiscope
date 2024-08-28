float screen_preview[SCREEN_PREVIEW_SIZE];

void run_screen_preview() {
	start_profile(__COUNTER__, __func__);
	for(uint8_t i = 0; i < SCREEN_PREVIEW_SIZE; i++){
		float max_col = 0.0;
		max_col = fmaxf(max_col, leds_last[i*2+0].r);
		max_col = fmaxf(max_col, leds_last[i*2+0].g);
		max_col = fmaxf(max_col, leds_last[i*2+0].b);
		max_col = fmaxf(max_col, leds_last[i*2+1].r);
		max_col = fmaxf(max_col, leds_last[i*2+1].g);
		max_col = fmaxf(max_col, leds_last[i*2+1].b);

		screen_preview[i] = max_col;
	}
	end_profile();
} 