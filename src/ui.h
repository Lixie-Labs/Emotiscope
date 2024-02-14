#define UI_HOLD_TIME_MS 1000

typedef enum {
    UI_NEEDLE_EVENT,
    UI_HUE_EVENT,
} ui_update_event;

ui_update_event last_update_type = UI_NEEDLE_EVENT;

float overlay_size = 0.0;
float overlay_size_target = 0.0;

uint32_t last_ui_update_ms = 0;

float ui_needle_position_raw = 0.0;
float ui_needle_position = 0.0;

void draw_ui_overlay(){
	// -----------------------------
	// Background
	apply_box_blur(leds, (NUM_LEDS>>1)*overlay_size, 13);

	draw_line(leds, 0, 0.5*overlay_size, {0.0,0.0,0.0}, 0.85*overlay_size);

	// -----------------------------
	// UI
	
	if(last_update_type == UI_NEEDLE_EVENT){
		for(uint16_t i = 0; i < 5; i++){
			draw_dot(leds, UI_1+i, {0.1,0.00,0.00}, 0 + ((0.5/4.0)*i)*overlay_size, overlay_size);
		}
		
		CRGBF gamma_corrected = {
			incandescent_lookup.r*incandescent_lookup.r,
			incandescent_lookup.g*incandescent_lookup.g,
			incandescent_lookup.b*incandescent_lookup.b,
		};

		draw_dot(leds, UI_NEEDLE, gamma_corrected, ui_needle_position*0.5*overlay_size, overlay_size);
	}

	// -----------------------------
	// Scaling / time

	if(t_now_ms - last_ui_update_ms >= UI_HOLD_TIME_MS){
		overlay_size_target = 0.0;
	}

	overlay_size = overlay_size_target * 0.05 + overlay_size * 0.95;

	ui_needle_position = ui_needle_position*0.75 + ui_needle_position_raw *0.25;
}

void update_ui(float new_value, ui_update_event update_type){
	last_ui_update_ms = t_now_ms;
	last_update_type = update_type;

	if(update_type == UI_NEEDLE_EVENT){
		ui_needle_position_raw = new_value;
	}

	overlay_size_target = 1.0;
}