extern float lpf_drag;
float breath_pos = PI * 0.75;

float standby_brightness = 1.0;

void run_standby(){
	static uint32_t led = 0;
	led++;

	clear_display();

	breath_pos += 0.005;

	float breath = (cos(breath_pos)*0.5+0.5);
	breath *= breath;
	float dot_pos = breath * 0.5 + 0.5;

	float dot_brightness = 0.2 + (breath*breath)*0.8;
	dot_brightness *= standby_brightness;

	CRGBF dot_color = {
		incandescent_lookup.r * incandescent_lookup.r,
		incandescent_lookup.g * incandescent_lookup.g,
		incandescent_lookup.b * incandescent_lookup.b,
	};

	draw_dot(leds, SCREENSAVER_1, dot_color,     dot_pos, dot_brightness);
	draw_dot(leds, SCREENSAVER_2, dot_color, 1.0-dot_pos, dot_brightness);

	if(standby_brightness > 0.00001){
		standby_brightness *= 0.999;
		lpf_drag = 1.0;
	}
}

void toggle_standby(){
	EMOTISCOPE_ACTIVE = !EMOTISCOPE_ACTIVE;
	breath_pos = PI * 0.75;
	standby_brightness = 1.0;

	lpf_drag = 1.0;	
}