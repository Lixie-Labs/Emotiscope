uint8_t screen_preview[SCREEN_PREVIEW_SIZE*3];

void run_screen_preview() {
	start_profile(__COUNTER__, __func__);
	for(uint8_t i = 0; i < SCREEN_PREVIEW_SIZE; i++){
		CRGBF color_a = led_preview[i*2+0];
		CRGBF color_b = led_preview[i*2+1];
		CRGBF color_out = {
			clip_float(fmaxf(color_a.r, color_b.r)),
			clip_float(fmaxf(color_a.g, color_b.g)),
			clip_float(fmaxf(color_a.b, color_b.b))
		};

		color_out.r = sqrtf(color_out.r);
		color_out.g = sqrtf(color_out.g);
		color_out.b = sqrtf(color_out.b);

		screen_preview[i*3+0] = (uint8_t)(color_out.r * 255);
		screen_preview[i*3+1] = (uint8_t)(color_out.g * 255);
		screen_preview[i*3+2] = (uint8_t)(color_out.b * 255);
	}
	end_profile();
}