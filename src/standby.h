extern float lpf_drag;
float breath_pos = PI * 0.75;

float standby_brightness = 1.0;
float standby_breath = 0.0;

void run_standby(){
	static uint32_t led = 0;
	led++;

	clear_display();

	breath_pos += 0.005;

	standby_breath = (cos(breath_pos)*0.5+0.5);
	standby_breath *= standby_breath;
	float dot_pos = standby_breath * 0.5 + 0.5;

	float dot_brightness = 0.6 + (standby_breath*standby_breath)*0.4;
	dot_brightness *= standby_brightness;

	CRGBF dot_color = {
		incandescent_lookup.r * incandescent_lookup.r,
		incandescent_lookup.g * incandescent_lookup.g,
		incandescent_lookup.b * incandescent_lookup.b,
	};

	if(standby_brightness > 0.00001){
		standby_brightness *= 0.999;
		lpf_drag = 1.0;

		draw_dot(leds, SLEEP_1, dot_color,     dot_pos, dot_brightness);
		draw_dot(leds, SLEEP_2, dot_color, 1.0-dot_pos, dot_brightness);
	}
}

void toggle_standby(){
	EMOTISCOPE_ACTIVE = !EMOTISCOPE_ACTIVE;
	breath_pos = PI * 0.75;
	standby_brightness = 1.0;
	standby_breath = 0.0;

	lpf_drag = 1.0;	
}