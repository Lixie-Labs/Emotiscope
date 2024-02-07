void draw_debug(){
	CRGBF colors[3] = {
		{1,0,0},
		{0,1,0},
		{0,0,1}
	};

	uint8_t color_index = 0;
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float progress = float(i) / (NUM_LEDS);
		progress *= progress;

		leds[i] = {
			colors[color_index].r*progress,
			colors[color_index].g*progress,
			colors[color_index].b*progress
		};

		//color_index += 1;
		if(color_index >= 3){
			color_index = 0;
		}
	}
}