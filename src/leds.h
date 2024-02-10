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

#define REFERENCE_FPS 100

CRGBF WHITE = {1.0, 1.0, 1.0};
CRGBF BLACK = {0.0, 0.0, 0.0};

CRGBF leds[NUM_LEDS]; // 32-bit image buffer

CRGBF leds_temp[NUM_LEDS]; // for temporary copies of the image buffer (scaling)

CRGBF leds_last[NUM_LEDS];

CRGBF leds_smooth[NUM_LEDS];

#define MAX_DOTS 192 
#define NUM_RESERVED 64 // TODO: implement reserved dots at the end of the array with an enum to name them
                        // Some can be specfically reserved for UI like tuning needles and the screensaver
fx_dot fx_dots[MAX_DOTS];

float rendered_debug_value = 0.0;

CRGBF incandescent_lookup = {1.0000, 0.4453, 0.1562};

float note_colors[12] = {0.0000, 0.0833, 0.1666, 0.2499, 0.3333, 0.4166,
						 0.4999, 0.5833, 0.6666, 0.7499, 0.8333, 0.9166};

// 256 pre-calculated values for full-saturated HSV lookup
const uint8_t hsv_lookup[256][3] = {
	{255,0,0},{253,2,0},{250,5,0},{247,8,0},{245,10,0},{242,13,0},{239,16,0},{237,18,0},{234,21,0},{231,24,0},{229,26,0},{226,29,0},{223,32,0},{221,34,0},{218,37,0},{215,40,0},{212,43,0},{210,45,0},{207,48,0},{204,51,0},{202,53,0},{199,56,0},{196,59,0},{194,61,0},{191,64,0},{188,67,0},{186,69,0},{183,72,0},{180,75,0},{178,77,0},{175,80,0},{172,83,0},{171,85,0},{171,87,0},{171,90,0},{171,93,0},{171,95,0},{171,98,0},{171,101,0},{171,103,0},{171,106,0},{171,109,0},{171,111,0},{171,114,0},{171,117,0},{171,119,0},{171,122,0},{171,125,0},{171,128,0},{171,130,0},{171,133,0},{171,136,0},{171,138,0},{171,141,0},{171,144,0},{171,146,0},{171,149,0},{171,152,0},{171,154,0},{171,157,0},{171,160,0},{171,162,0},{171,165,0},{171,168,0},{171,170,0},{166,172,0},{161,175,0},{155,178,0},{150,180,0},{145,183,0},{139,186,0},{134,188,0},{129,191,0},{123,194,0},{118,196,0},{113,199,0},{107,202,0},{102,204,0},{97,207,0},{91,210,0},{86,213,0},{81,215,0},{75,218,0},{70,221,0},{65,223,0},{59,226,0},{54,229,0},{49,231,0},{43,234,0},{38,237,0},{33,239,0},{27,242,0},{22,245,0},{17,247,0},{11,250,0},{6,253,0},{0,255,0},{0,253,2},{0,250,5},{0,247,8},{0,245,10},{0,242,13},{0,239,16},{0,237,18},{0,234,21},{0,231,24},{0,229,26},{0,226,29},{0,223,32},{0,221,34},{0,218,37},{0,215,40},{0,212,43},{0,210,45},{0,207,48},{0,204,51},{0,202,53},{0,199,56},{0,196,59},{0,194,61},{0,191,64},{0,188,67},{0,186,69},{0,183,72},{0,180,75},{0,178,77},{0,175,80},{0,172,83},{0,171,85},{0,166,90},{0,161,95},{0,155,101},{0,150,106},{0,145,111},{0,139,117},{0,134,122},{0,129,127},{0,123,133},{0,118,138},{0,113,143},{0,107,149},{0,102,154},{0,97,159},{0,91,165},{0,86,170},{0,81,175},{0,75,181},{0,70,186},{0,65,191},{0,59,197},{0,54,202},{0,49,207},{0,43,213},{0,38,218},{0,33,223},{0,27,229},{0,22,234},{0,17,239},{0,11,245},{0,6,250},{0,0,255},{2,0,253},{5,0,250},{8,0,247},{10,0,245},{13,0,242},{16,0,239},{18,0,237},{21,0,234},{24,0,231},{26,0,229},{29,0,226},{32,0,223},{34,0,221},{37,0,218},{40,0,215},{43,0,212},{45,0,210},{48,0,207},{51,0,204},{53,0,202},{56,0,199},{59,0,196},{61,0,194},{64,0,191},{67,0,188},{69,0,186},{72,0,183},{75,0,180},{77,0,178},{80,0,175},{83,0,172},{85,0,171},{87,0,169},{90,0,166},{93,0,163},{95,0,161},{98,0,158},{101,0,155},{103,0,153},{106,0,150},{109,0,147},{111,0,145},{114,0,142},{117,0,139},{119,0,137},{122,0,134},{125,0,131},{128,0,128},{130,0,126},{133,0,123},{136,0,120},{138,0,118},{141,0,115},{144,0,112},{146,0,110},{149,0,107},{152,0,104},{154,0,102},{157,0,99},{160,0,96},{162,0,94},{165,0,91},{168,0,88},{170,0,85},{172,0,83},{175,0,80},{178,0,77},{180,0,75},{183,0,72},{186,0,69},{188,0,67},{191,0,64},{194,0,61},{196,0,59},{199,0,56},{202,0,53},{204,0,51},{207,0,48},{210,0,45},{213,0,42},{215,0,40},{218,0,37},{221,0,34},{223,0,32},{226,0,29},{229,0,26},{231,0,24},{234,0,21},{237,0,18},{239,0,16},{242,0,13},{245,0,10},{247,0,8},{250,0,5},{253,0,2}
};

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

// These dsps_***() functions are from the ESP-DSP Espressif library which seem to
// multiply arrays of floats faster than otherwise possible.
//
// There's a hardware accelerated FFT function in there that I'm not even using
// because the current method of having 128 instances of the Goertzel algorithm at once
// is still more flexible for getting good spectral shows.
//
// (64 are musical notes, the other 64 are tempi)
//
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

CRGBF desaturate(struct CRGBF input_color, float amount) {
    float luminance = 0.2126 * input_color.r + 0.7152 * input_color.g + 0.0722 * input_color.b;
    float amount_inv = 1.0 - amount;

    CRGBF output;
    output.r = input_color.r * amount_inv + luminance * amount;
    output.g = input_color.g * amount_inv + luminance * amount;
    output.b = input_color.b * amount_inv + luminance * amount;

    return output;
}

CRGBF hsv(float h, float s, float v) {
	h = fmodf(h, 1.0f);
	while (h < 0.0f) h += 1.0f;

	v = clip_float(v);
	uint8_t h_8_bit = h * 255.0f;

	CRGBF col = {(hsv_lookup[h_8_bit][0] / 255.0f) * v, (hsv_lookup[h_8_bit][1] / 255.0f) * v,
				 (hsv_lookup[h_8_bit][2] / 255.0f) * v};

	col = desaturate(col, 1.0-s);

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
	float alpha = 1.0 - expf(-6.28318530718 * cutoff_frequency / FPS_GPU);
	float alpha_inv = 1.0 - alpha;

	// Crasy fast SIMD-style math possible with the S3
	scale_CRGBF_array_by_constant(leds, alpha, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds_last, alpha_inv, NUM_LEDS);

	add_CRGBF_arrays(leds, leds_last, NUM_LEDS);

	memcpy(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);
}

void apply_brightness() {
	float brightness_val = 0.25+configuration.brightness*0.75;

	scale_CRGBF_array_by_constant(leds, brightness_val*brightness_val, NUM_LEDS);
}

void clear_display(){
	memset(leds, 0, sizeof(CRGBF)*NUM_LEDS);
}