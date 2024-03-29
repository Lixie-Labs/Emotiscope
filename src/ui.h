#define UI_HOLD_TIME_MS 1000

typedef enum {
	UI_SHOW_EVENT,
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
	// Blur background
	apply_box_blur(leds, (NUM_LEDS>>1)*overlay_size, 13);

	// -----------------------------
	// Darken background
	draw_line(leds, 0, 0.5*overlay_size, {0,0,0}, 0.9*overlay_size);

	// -----------------------------
	// Draw UI
	if(last_update_type == UI_NEEDLE_EVENT){
		CRGBF back_color = hsv(0.870, 1.0, 0.05);
		draw_line(leds, 0, ui_needle_position*0.5*overlay_size, back_color, 0.98*overlay_size);
	
		CRGBF dot_color = hsv(0.814, 1.0, 1.0);
		for(uint16_t i = 0; i < 5; i++){
			draw_dot(leds, UI_1+i, dot_color, 0 + ((0.5/4.0)*i)*overlay_size, overlay_size*0.15);
		}
		
		CRGBF gamma_corrected = {
			incandescent_lookup.r*incandescent_lookup.r,
			incandescent_lookup.g*incandescent_lookup.g,
			incandescent_lookup.b*incandescent_lookup.b,
		};

		draw_dot(leds, UI_NEEDLE, gamma_corrected, ui_needle_position*0.5*overlay_size, overlay_size);
	}

	// -----------------------------
	// Handle scaling / time

	if(t_now_ms - last_ui_update_ms >= UI_HOLD_TIME_MS){
		if(slider_touch_active == false){
			overlay_size_target = 0.0;
		}
	}

	overlay_size = overlay_size_target * 0.05 + overlay_size * 0.95;

	ui_needle_position = ui_needle_position*0.75 + ui_needle_position_raw *0.25;
}

void update_ui(ui_update_event update_type, float new_value = 0.0){
	last_ui_update_ms = t_now_ms;
	last_update_type = update_type;

	if(update_type == UI_NEEDLE_EVENT){
		ui_needle_position_raw = new_value;
	}

	overlay_size_target = 1.0;
}