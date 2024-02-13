#define STANDBY_BRIGHTNESS 0.25

extern float lpf_drag;
float breath_pos = 0.000;

void run_standby(){
	static uint32_t led = 0;
	led++;

	clear_display();

	breath_pos += 0.005;

	float breath = (sin(breath_pos)*0.5+0.5);
	breath *= breath;
	float dot_pos = breath * 0.5 + 0.5;

	float dot_brightness = 0.2 + (breath*breath)*0.8;
	dot_brightness *= STANDBY_BRIGHTNESS;

	CRGBF dot_color = {
		incandescent_lookup.r * incandescent_lookup.r,
		incandescent_lookup.g * incandescent_lookup.g,
		incandescent_lookup.b * incandescent_lookup.b,
	};

	draw_dot(leds, SCREENSAVER_1, dot_color,     dot_pos, dot_brightness);
	draw_dot(leds, SCREENSAVER_2, dot_color, 1.0-dot_pos, dot_brightness);
}

void toggle_standby(){
	EMOTISCOPE_ACTIVE = !EMOTISCOPE_ACTIVE;
	lpf_drag = 1.0;
}