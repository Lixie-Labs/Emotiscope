#define MAX_TOGGLES 32

toggle toggles[MAX_TOGGLES];
uint16_t toggles_active = 0;

void clear_toggles() {
	for (uint16_t i = 0; i < MAX_TOGGLES; i++) {
		memset(toggles[i].name, 0, 32);
	}
	toggles_active = 0;
}

bool register_toggle(char* name) {
	bool register_success = false;
	for (uint16_t i = 0; i < MAX_TOGGLES; i++) {
		if (toggles[i].name[0] == 0) {	// Unoccupied slot
			memcpy(toggles[i].name, name, strlen(name));
			toggles_active += 1;
			register_success = true;
			break;
		}
	}

	return register_success;
}

void load_toggles_relevant_to_mode(int16_t mode_index) {
	clear_toggles();

	if (mode_index == 0) {  // Analog
		register_toggle("mirror_mode");
	}
	else if (mode_index == 1) {  // Spectrum
		register_toggle("mirror_mode");
	}
	else if (mode_index == 2) {  // Octave
		register_toggle("mirror_mode");
	}
	else if (mode_index == 3) {  // Metronome
		register_toggle("mirror_mode");
	}
	else if (mode_index == 4) {  // Spectrum + Beat
		register_toggle("mirror_mode");
	}
	else if (mode_index == 5) {  // Hype
		register_toggle("mirror_mode");
	}
	else if (mode_index == 6) {  // Bloom
		register_toggle("mirror_mode");
	}

	else{
		wtf_error();
	}
}