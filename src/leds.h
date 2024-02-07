// ------------------------------------
//  _              _             _
// | |            | |           | |
// | |   ___    __| |  ___      | |__
// | |  / _ \  / _` | / __|     | '_ \ 
// | | |  __/ | (_| | \__ \  _  | | | |
// |_|  \___|  \__,_| |___/ (_) |_| |_|
//
// Functions for manipulating and updating WS2812Bs using a custom
// floating-point "CRGBF" format.

#define DATA_PIN_1 13
#define DATA_PIN_2 12
#define LED_TYPE NEOPIXEL
#define COLOR_ORDER GRB
#define NUM_LEDS 128

#define MAX_DOTS 256

#define REFERENCE_FPS 100

CRGBF WHITE = {1.0, 1.0, 1.0};
CRGBF BLACK = {0.0, 0.0, 0.0};

CRGBF leds[NUM_LEDS];
CRGBF leds_temp[NUM_LEDS];
CRGBF leds_last[NUM_LEDS];
CRGBF leds_smooth[NUM_LEDS];

//CRGB leds_8[NUM_LEDS];

fx_dot fx_dots[MAX_DOTS];

float rendered_debug_value = 0.0;

CRGBF incandescent_lookup = {1.0000, 0.4453, 0.1562};

float note_colors[12] = {0.0000, 0.0833, 0.1666, 0.2499, 0.3333, 0.4166,
						 0.4999, 0.5833, 0.6666, 0.7499, 0.8333, 0.9166};

void init_leds() {
	//rmt_tx_init(RMT_CHANNEL_0, RMT_A_GPIO );
	//rmt_tx_init(RMT_CHANNEL_1, RMT_B_GPIO );
}

void save_leds_to_temp() {
	// uint16_t profiler_index = start_function_timing(__func__);
	memcpy(leds_temp, leds, sizeof(CRGBF) * NUM_LEDS);
	// end_function_timing(profiler_index);
}

void load_leds_from_temp() {
	// uint16_t profiler_index = start_function_timing(__func__);
	memcpy(leds, leds_temp, sizeof(CRGBF) * NUM_LEDS);
	// end_function_timing(profiler_index);
}

void multiply_CRGBF_array_by_LUT(CRGBF* input, CRGBF LUT, uint16_t array_length) {
	float* ptr = (float*)input;

	dsps_mulc_f32(ptr + 0, ptr + 0, array_length, LUT.r, 3, 3);
	dsps_mulc_f32(ptr + 1, ptr + 1, array_length, LUT.g, 3, 3);
	dsps_mulc_f32(ptr + 2, ptr + 2, array_length, LUT.b, 3, 3);
}

void scale_CRGBF_array_by_constant(CRGBF* input, float scale_value, uint16_t array_length) {
	float* ptr = (float*)input;
	dsps_mulc_f32(ptr, ptr, array_length * 3, scale_value, 1, 1);
}

void add_CRGBF_arrays(CRGBF* a, CRGBF* b, uint16_t array_length) {
	float* ptr_a = (float*)a;
	float* ptr_b = (float*)b;

	// Stores result in "a" array
	dsps_add_f32(ptr_a, ptr_b, ptr_a, array_length * 3, 1, 1, 1);
}

void smooth_led_output(float blend_strength) {
	if (blend_strength > 0.0) {
		float update_ratio = 1.0 - blend_strength;
		float update_ratio_inv = blend_strength;

		for (uint16_t i = 0; i < NUM_LEDS; i += 4) {
			leds_smooth[i + 0].r = leds_smooth[i + 0].r * (update_ratio_inv) + leds[i + 0].r * (update_ratio);
			leds_smooth[i + 0].g = leds_smooth[i + 0].g * (update_ratio_inv) + leds[i + 0].g * (update_ratio);
			leds_smooth[i + 0].b = leds_smooth[i + 0].b * (update_ratio_inv) + leds[i + 0].b * (update_ratio);

			leds_smooth[i + 1].r = leds_smooth[i + 1].r * (update_ratio_inv) + leds[i + 1].r * (update_ratio);
			leds_smooth[i + 1].g = leds_smooth[i + 1].g * (update_ratio_inv) + leds[i + 1].g * (update_ratio);
			leds_smooth[i + 1].b = leds_smooth[i + 1].b * (update_ratio_inv) + leds[i + 1].b * (update_ratio);

			leds_smooth[i + 2].r = leds_smooth[i + 2].r * (update_ratio_inv) + leds[i + 2].r * (update_ratio);
			leds_smooth[i + 2].g = leds_smooth[i + 2].g * (update_ratio_inv) + leds[i + 2].g * (update_ratio);
			leds_smooth[i + 2].b = leds_smooth[i + 2].b * (update_ratio_inv) + leds[i + 2].b * (update_ratio);

			leds_smooth[i + 3].r = leds_smooth[i + 3].r * (update_ratio_inv) + leds[i + 3].r * (update_ratio);
			leds_smooth[i + 3].g = leds_smooth[i + 3].g * (update_ratio_inv) + leds[i + 3].g * (update_ratio);
			leds_smooth[i + 3].b = leds_smooth[i + 3].b * (update_ratio_inv) + leds[i + 3].b * (update_ratio);
		}
	}
	else {
		memcpy(leds_smooth, leds, sizeof(CRGBF) * NUM_LEDS);
	}
}

void clip_leds() {
	// Loop unroll for speed
	for (uint16_t i = 0; i < NUM_LEDS; i += 4) {
		leds[i + 0].r = clip_float(leds[i + 0].r);
		leds[i + 0].g = clip_float(leds[i + 0].g);
		leds[i + 0].b = clip_float(leds[i + 0].b);

		leds[i + 1].r = clip_float(leds[i + 1].r);
		leds[i + 1].g = clip_float(leds[i + 1].g);
		leds[i + 1].b = clip_float(leds[i + 1].b);

		leds[i + 2].r = clip_float(leds[i + 2].r);
		leds[i + 2].g = clip_float(leds[i + 2].g);
		leds[i + 2].b = clip_float(leds[i + 2].b);

		leds[i + 3].r = clip_float(leds[i + 3].r);
		leds[i + 3].g = clip_float(leds[i + 3].g);
		leds[i + 3].b = clip_float(leds[i + 3].b);
	}
}

void quantize_color() {
	const float dither_table[4] = {0.25, 0.50, 0.75, 1.00};
	static uint8_t dither_step = 0;
	dither_step++;

	float decimal_r; float decimal_g; float decimal_b;
	uint8_t whole_r; uint8_t whole_g; uint8_t whole_b;
	float   fract_r; float   fract_g; float   fract_b;

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		// RED #####################################################
		decimal_r = leds[i].r * 254;
		whole_r = decimal_r;
		fract_r = decimal_r - whole_r;
		raw_led_data[3*i+1] = whole_r + (fract_r >= dither_table[(dither_step + i) % 4]);
		
		// GREEN #####################################################
		decimal_g = leds[i].g * 254;
		whole_g = decimal_g;
		fract_g = decimal_g - whole_g;
		raw_led_data[3*i+0] = whole_g + (fract_g >= dither_table[(dither_step + i) % 4]);

		// BLUE #####################################################
		decimal_b = leds[i].b * 254;
		whole_b = decimal_b;
		fract_b = decimal_b - whole_b;
		raw_led_data[3*i+2] = whole_b + (fract_b >= dither_table[(dither_step + i) % 4]);
	}
}

void quantize_color_rough() {
	memset(raw_led_data, 0, sizeof(raw_led_data));

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		raw_led_data[3*i+0] = leds[i].g*254;
		raw_led_data[3*i+1] = leds[i].r*254;
		raw_led_data[3*i+2] = leds[i].b*254;
	}
}

CRGBF hsv(float h, float s, float v) {
	h = fmodf(h, 1.0f);
	while (h < 0.0f) h += 1.0f;

	//v = clip_float(v);

	// TODO: Re-implement HSV calculation without FastLED
	/*
	CRGB base_color = CHSV(h_8_bit, s_8_bit, 255);

	CRGBF col = {(base_color.r / 255.0f) * v, (base_color.g / 255.0f) * v,
				 (base_color.b / 255.0f) * v};
	*/
	CRGBF col = {
		1.0 * v,
		0.0 * v, 
		0.0 * v
	};

	col.r = min(col.r, 1.0f);
	col.g = min(col.g, 1.0f);
	col.b = min(col.b, 1.0f);

	return col;
}

void apply_incandescent_filter(float mix) {
	uint32_t t_start_cycles = ESP.getCycleCount();

	float mix_inv = 1.0 - mix;
	save_leds_to_temp();
	multiply_CRGBF_array_by_LUT(leds_temp, incandescent_lookup, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds_temp, mix, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds, mix_inv, NUM_LEDS);
	add_CRGBF_arrays(leds, leds_temp, NUM_LEDS);

	uint32_t t_end_cycles = ESP.getCycleCount();
	volatile uint32_t t_total_cycles = t_end_cycles - t_start_cycles;

}

void save_leds_to_last() {
	memcpy(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);
}

CRGBF mix(CRGBF color_1, CRGBF color_2, float amount) {
	CRGBF out_color = {
		color_1.r * (1.0 - amount) + color_2.r * (amount),
		color_1.g * (1.0 - amount) + color_2.g * (amount),
		color_1.b * (1.0 - amount) + color_2.b * (amount),
	};

	return out_color;
}

void apply_scaling_mode() {
	// Work using this copy
	save_leds_to_temp();

	// Nearest Neighbor Stretch
	// for(uint16_t i = 0; i < NUM_LEDS; i++){
	//  leds[i] = leds_temp[i>>1];
	//}
	//*/

	// Linear Stretch
	/*
	for (uint16_t i = 0; i < NUM_LEDS - 1; i++) {
	  float i_scaled = i / 2.0;
	  uint16_t i_scaled_whole = i_scaled;
	  float i_scaled_fract = i_scaled - i_scaled_whole;

	  leds[i] = mix(leds_temp[i_scaled_whole], leds_temp[i_scaled_whole + 1],
	i_scaled_fract);
	}
	leds[NUM_LEDS - 1] = leds[NUM_LEDS - 2];
	*/

	// Mirror Mode
	for (uint16_t i = 0; i < (NUM_LEDS >> 1); i++) {  // Squash to one half
		int16_t fetch_led = i << 1;
		leds[(NUM_LEDS >> 1) + i].r = leds_temp[fetch_led + 0].r * 0.5 + leds_temp[fetch_led + 1].r * 0.5;
		leds[(NUM_LEDS >> 1) + i].g = leds_temp[fetch_led + 0].g * 0.5 + leds_temp[fetch_led + 1].g * 0.5;
		leds[(NUM_LEDS >> 1) + i].b = leds_temp[fetch_led + 0].b * 0.5 + leds_temp[fetch_led + 1].b * 0.5;

		leds[((NUM_LEDS >> 1) - 1) - i] = leds[(NUM_LEDS >> 1) + i];
	}

}

void rough_mirror_screen() {
	save_leds_to_temp();
	uint16_t half_width = NUM_LEDS >> 1;
	for (uint16_t i = 0; i < half_width; i++) {
		CRGBF sample_point = leds_temp[i << 1];
		leds[half_width + i] = sample_point;
		leds[(half_width - 1) - i] = sample_point;
	}
}

CRGBF add(CRGBF color_1, CRGBF color_2, float add_amount = 1.0) {
	CRGBF out_color = {
		color_1.r + color_2.r * (add_amount),
		color_1.g + color_2.g * (add_amount),
		color_1.b + color_2.b * (add_amount),
	};

	out_color.r = min(1.0f, out_color.r);
	out_color.g = min(1.0f, out_color.g);
	out_color.b = min(1.0f, out_color.b);

	return out_color;
}

void apply_video_feedback() {
	// Work using the last frame
	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		// leds[i] = mix(leds[i], leds_last[i], 0.5);
		leds[i] = add(leds[i], leds_last[i], 0.65);
	}
}

void draw_line(CRGBF* layer, float x1, float x2, CRGBF color, float opacity) {
	bool lighten = true;
	if (color.r == 0 && color.g == 0 && color.b == 0) {
		lighten = false;
	}

	x1 *= (float)(NUM_LEDS - 1);
	x2 *= (float)(NUM_LEDS - 1);

	if (x1 > x2) {	// Ensure x1 <= x2
		float temp = x1;
		x1 = x2;
		x2 = temp;
	}

	float ix1 = floor(x1);
	float ix2 = ceil(x2);

	// start pixel
	if (ix1 >= 0 && ix1 < NUM_LEDS) {
		float coverage = 1.0 - (x1 - ix1);
		float mix = opacity * coverage;

		if (lighten == true) {
			layer[uint16_t(ix1)].r += color.r * mix;
			layer[uint16_t(ix1)].g += color.g * mix;
			layer[uint16_t(ix1)].b += color.b * mix;
		}
		else {
			layer[uint16_t(ix1)].r = layer[uint16_t(ix1)].r * (1.0 - mix) + color.r * mix;
			layer[uint16_t(ix1)].g = layer[uint16_t(ix1)].g * (1.0 - mix) + color.g * mix;
			layer[uint16_t(ix1)].b = layer[uint16_t(ix1)].b * (1.0 - mix) + color.b * mix;
		}
	}

	// end pixel
	if (ix2 >= 0 && ix2 < 128) {
		float coverage = x2 - floor(x2);
		float mix = opacity * coverage;

		if (lighten == true) {
			layer[uint16_t(ix2)].r += color.r * mix;
			layer[uint16_t(ix2)].g += color.g * mix;
			layer[uint16_t(ix2)].b += color.b * mix;
		}
		else {
			layer[uint16_t(ix2)].r = layer[uint16_t(ix2)].r * (1.0 - mix) + color.r * mix;
			layer[uint16_t(ix2)].g = layer[uint16_t(ix2)].g * (1.0 - mix) + color.g * mix;
			layer[uint16_t(ix2)].b = layer[uint16_t(ix2)].b * (1.0 - mix) + color.b * mix;
		}
	}

	// pixels in between
	for (float i = ix1 + 1; i < ix2; i++) {
		if (i >= 0 && i < NUM_LEDS) {
			layer[uint16_t(i)].r += color.r * opacity;
			layer[uint16_t(i)].g += color.g * opacity;
			layer[uint16_t(i)].b += color.b * opacity;

			if (lighten == true) {
				layer[uint16_t(i)].r += color.r * opacity;
				layer[uint16_t(i)].g += color.g * opacity;
				layer[uint16_t(i)].b += color.b * opacity;
			}
			else {
				layer[uint16_t(i)].r = layer[uint16_t(i)].r * (1.0 - opacity) + color.r * opacity;
				layer[uint16_t(i)].g = layer[uint16_t(i)].g * (1.0 - opacity) + color.g * opacity;
				layer[uint16_t(i)].b = layer[uint16_t(i)].b * (1.0 - opacity) + color.b * opacity;
			}
		}
	}
}

void draw_dot(CRGBF* layer, uint16_t fx_dots_slot, CRGBF color, float position, float opacity = 1.0) {
	fx_dots[fx_dots_slot].position_past = fx_dots[fx_dots_slot].position;
	fx_dots[fx_dots_slot].position = position;

	float position_distance = fabs(position - fx_dots[fx_dots_slot].position_past);
	if (position_distance < 1.0) {
		position_distance = 1.0;
	}

	float spread_brightness = 1.0 / position_distance;
	if (spread_brightness > 1.0) {
		spread_brightness = 1.0;
	}

	draw_line(layer, position, fx_dots[fx_dots_slot].position_past, color, spread_brightness * opacity);
}

void render_debug_value(uint32_t t_now_ms) {
	static float value_last = 0;
	static uint32_t last_update_time;
	static float opacity_target = 0.0;
	static float opacity = 0.0;
	static float rendered_debug_value_smooth = 0.0;
	
	rendered_debug_value_smooth = rendered_debug_value_smooth*0.75 + rendered_debug_value*0.25;

	if(rendered_debug_value != value_last){
		last_update_time = t_now_ms;
		opacity_target = 1.0;
		value_last = rendered_debug_value;
	}

	if(t_now_ms - last_update_time >= 1000){
		opacity_target = 0.0;
	}

	opacity = opacity*0.95 + opacity_target*0.05;

	CRGBF gamma_corrected = {
		incandescent_lookup.r*incandescent_lookup.r,
		incandescent_lookup.g*incandescent_lookup.g,
		incandescent_lookup.b*incandescent_lookup.b,
	};

	draw_dot(leds, MAX_DOTS - 5, gamma_corrected, clip_float(rendered_debug_value_smooth), opacity);
}

void apply_image_lpf(float cutoff_frequency) {
	// Calculate alpha once and pass it to the low_pass_image function
	float alpha = 1.0 - expf(-6.28318530718 * cutoff_frequency / FPS_GPU);
	float alpha_inv = 1.0 - alpha;

	scale_CRGBF_array_by_constant(leds, alpha, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds_last, alpha_inv, NUM_LEDS);
	add_CRGBF_arrays(leds, leds_last, NUM_LEDS);

	/*
	for(uint16_t i = 0; i < NUM_LEDS; i += 1){
	  leds[i].r = (alpha_inv) * leds_last[i].r + alpha * leds[i].r;
	  leds[i].g = (alpha_inv) * leds_last[i].g + alpha * leds[i].g;
	  leds[i].b = (alpha_inv) * leds_last[i].b + alpha * leds[i].b;
	}
	*/

	memcpy(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);
}

void apply_brightness() {
	float brightness_val = 0.25+configuration.brightness*0.75;

	scale_CRGBF_array_by_constant(leds, brightness_val*brightness_val, NUM_LEDS);
}

void clear_display(){
	memset(leds, 0, sizeof(CRGBF)*NUM_LEDS);
}