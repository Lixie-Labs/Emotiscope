#define MAX_MENU_TOGGLES 32

menu_toggle menu_toggles[MAX_MENU_TOGGLES];
uint16_t menu_toggles_active = 0;

void clear_menu_toggles() {
	for (uint16_t i = 0; i < MAX_MENU_TOGGLES; i++) {
		memset(menu_toggles[i].name, 0, 32);
	}
	menu_toggles_active = 0;
}

bool register_menu_toggle(char* name) {
	bool register_success = false;
	for (uint16_t i = 0; i < MAX_MENU_TOGGLES; i++) {
		if (menu_toggles[i].name[0] == 0) {	// Unoccupied slot
			memcpy(menu_toggles[i].name, name, strlen(name));
			menu_toggles_active += 1;
			register_success = true;
			break;
		}
	}

	return register_success;
}

void load_menu_toggles() {
	clear_menu_toggles();

	register_menu_toggle("screensaver");
	register_menu_toggle("invert_color_range");
	register_menu_toggle("temporal_dithering");
}