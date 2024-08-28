uint8_t screen_preview[SCREEN_PREVIEW_SIZE*3];

void run_screen_preview() {
	start_profile(__COUNTER__, __func__);
	for(uint8_t i = 0; i < SCREEN_PREVIEW_SIZE; i++){
		CRGBF color_a = leds_last[i*2+0];
		CRGBF color_b = leds_last[i*2+1];
		CRGBF color_out = {
			clip_float((color_a.r + color_b.r) * 0.5),
			clip_float((color_a.g + color_b.g) * 0.5),
			clip_float((color_a.b + color_b.b) * 0.5)
		};

		screen_preview[i*3+0] = (uint8_t)(color_out.r * 255);
		screen_preview[i*3+1] = (uint8_t)(color_out.g * 255);
		screen_preview[i*3+2] = (uint8_t)(color_out.b * 255);
	}
	end_profile();
}