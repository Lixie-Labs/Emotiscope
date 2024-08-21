void draw_temp(){
	memset(leds, 0, sizeof(CRGBF) * NUM_LEDS);

	for(uint16_t i = 0; i < 8; i++){
		if (i < 4) {
			leds[i+16+32] = incandescent_lookup;
		}
		else {
			leds[i+24+32] = incandescent_lookup;
		}
	}
}