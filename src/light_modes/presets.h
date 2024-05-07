void draw_presets(){
	// Draw a dot moving left and right with a sine wave
	// The dot is colored red

	draw_dot(leds, NUM_RESERVED_DOTS+0, CRGBF(1.0, 0.0, 0.0), 0.5 + 0.5 * sin(millis() * 0.001), 1.0);
}