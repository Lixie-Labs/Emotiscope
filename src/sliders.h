#define MAX_SLIDERS 32

slider sliders[MAX_SLIDERS];
uint16_t sliders_active = 0;

void clear_sliders() {
	for (uint16_t i = 0; i < MAX_SLIDERS; i++) {
		memset(sliders[i].name, 0, 32);
		sliders[i].slider_min = 0.000;
		sliders[i].slider_max = 1.000;
		sliders[i].slider_step = 0.001;
	}
	sliders_active = 0;
}

bool register_slider(char* name, float slider_min, float slider_max, float slider_step) {
	bool register_success = false;
	for (uint16_t i = 0; i < MAX_SLIDERS; i++) {
		if (sliders[i].name[0] == 0) {	// Unoccupied slot
			memcpy(sliders[i].name, name, strlen(name));
			sliders[i].slider_min = slider_min;
			sliders[i].slider_max = slider_max;
			sliders[i].slider_step = slider_step;
			sliders_active += 1;
			register_success = true;
			break;
		}
	}

	return register_success;
}

void load_sliders_relevant_to_mode(int16_t mode_index) {
	clear_sliders();

	register_slider("brightness", 0.000, 1.000, 0.001);
	register_slider("melt", 0.000, 1.000, 0.001);

	if (mode_index == 0) {  // Analog
		register_slider("speed", 0.000, 1.000, 0.001);
	}
	else if (mode_index == 1) {  // Spectrum
		// No extra sliders yet
	}
	else if (mode_index == 2) {  // Octave
		// No extra sliders yet
	}
	else if (mode_index == 3) {  // Metronome
		// No extra sliders yet
	}
	else if (mode_index == 4) {  // Spectrum + Beat
		// No extra sliders yet
	}
	else if (mode_index == 5) {  // Hype
		// No extra sliders yet
	}
	else if (mode_index == 6) {  // Bloom
		register_slider("speed", 0.000, 1.000, 0.001);
	}
	else{
		// WTF happened that got you here?
	}

	register_slider("hue",          0.000, 1.000, 0.001);
	register_slider("hue_range",    0.000, 1.000, 0.001);
	register_slider("saturation",   0.000, 1.000, 0.001);
	register_slider("incandescent", 0.000, 1.000, 0.001);
	register_slider("background",   0.000, 1.000, 0.001);
}