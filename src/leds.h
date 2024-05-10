/*
------------------------------------
 _              _             _
| |            | |           | |
| |   ___    __| |  ___      | |__
| |  / _ \  / _` | / __|     | '_ \ 
| | |  __/ | (_| | \__ \  _  | | | |
|_|  \___|  \__,_| |___/ (_) |_| |_|

Functions for manipulating and updating WS2812Bs using a custom
floating-point "CRGBF" format.
*/

#define DATA_PIN_1 13
#define DATA_PIN_2 12
#define LED_TYPE NEOPIXEL
#define COLOR_ORDER GRB

#define REFERENCE_FPS 100

#define MAX_DOTS 384

CRGBF WHITE_BALANCE = { 1.0, 0.75, 0.60 };

typedef enum {
	UI_1, UI_2, UI_3, UI_4, UI_5, UI_6, UI_7, UI_8, UI_9, UI_10,
	UI_NEEDLE,
	SLEEP_1, SLEEP_2,
	SCREENSAVER_1, SCREENSAVER_2, SCREENSAVER_3, SCREENSAVER_4,
    NUM_RESERVED_DOTS
} reserved_dots_t;

fx_dot fx_dots[MAX_DOTS];

CRGBF WHITE = {1.0, 1.0, 1.0};
CRGBF BLACK = {0.0, 0.0, 0.0};

CRGBF leds[NUM_LEDS]; // 32-bit image buffer

CRGBF leds_scaled[NUM_LEDS]; // scaled up to 8-bit range, but still floating point

CRGBF leds_temp[NUM_LEDS]; // for temporary copies of the image buffer (scaling)

CRGBF leds_last[NUM_LEDS];

CRGBF leds_smooth[NUM_LEDS];

float rendered_debug_value = 0.0;

CRGBF incandescent_lookup = {sqrt(1.0000), sqrt(0.1982), sqrt(0.0244)};

float note_colors[12] = {0.0000, 0.0833, 0.1666, 0.2499, 0.3333, 0.4166,
						 0.4999, 0.5833, 0.6666, 0.7499, 0.8333, 0.9166};

// 256 pre-calculated values for full-saturated HSV lookup
const uint8_t hsv_lookup[256][3] = {
	{255,0,0},{253,2,0},{250,5,0},{247,8,0},{245,10,0},{242,13,0},{239,16,0},{237,18,0},{234,21,0},{231,24,0},{229,26,0},{226,29,0},{223,32,0},{221,34,0},{218,37,0},{215,40,0},{212,43,0},{210,45,0},{207,48,0},{204,51,0},{202,53,0},{199,56,0},{196,59,0},{194,61,0},{191,64,0},{188,67,0},{186,69,0},{183,72,0},{180,75,0},{178,77,0},{175,80,0},{172,83,0},{171,85,0},{171,87,0},{171,90,0},{171,93,0},{171,95,0},{171,98,0},{171,101,0},{171,103,0},{171,106,0},{171,109,0},{171,111,0},{171,114,0},{171,117,0},{171,119,0},{171,122,0},{171,125,0},{171,128,0},{171,130,0},{171,133,0},{171,136,0},{171,138,0},{171,141,0},{171,144,0},{171,146,0},{171,149,0},{171,152,0},{171,154,0},{171,157,0},{171,160,0},{171,162,0},{171,165,0},{171,168,0},{171,170,0},{166,172,0},{161,175,0},{155,178,0},{150,180,0},{145,183,0},{139,186,0},{134,188,0},{129,191,0},{123,194,0},{118,196,0},{113,199,0},{107,202,0},{102,204,0},{97,207,0},{91,210,0},{86,213,0},{81,215,0},{75,218,0},{70,221,0},{65,223,0},{59,226,0},{54,229,0},{49,231,0},{43,234,0},{38,237,0},{33,239,0},{27,242,0},{22,245,0},{17,247,0},{11,250,0},{6,253,0},{0,255,0},{0,253,2},{0,250,5},{0,247,8},{0,245,10},{0,242,13},{0,239,16},{0,237,18},{0,234,21},{0,231,24},{0,229,26},{0,226,29},{0,223,32},{0,221,34},{0,218,37},{0,215,40},{0,212,43},{0,210,45},{0,207,48},{0,204,51},{0,202,53},{0,199,56},{0,196,59},{0,194,61},{0,191,64},{0,188,67},{0,186,69},{0,183,72},{0,180,75},{0,178,77},{0,175,80},{0,172,83},{0,171,85},{0,166,90},{0,161,95},{0,155,101},{0,150,106},{0,145,111},{0,139,117},{0,134,122},{0,129,127},{0,123,133},{0,118,138},{0,113,143},{0,107,149},{0,102,154},{0,97,159},{0,91,165},{0,86,170},{0,81,175},{0,75,181},{0,70,186},{0,65,191},{0,59,197},{0,54,202},{0,49,207},{0,43,213},{0,38,218},{0,33,223},{0,27,229},{0,22,234},{0,17,239},{0,11,245},{0,6,250},{0,0,255},{2,0,253},{5,0,250},{8,0,247},{10,0,245},{13,0,242},{16,0,239},{18,0,237},{21,0,234},{24,0,231},{26,0,229},{29,0,226},{32,0,223},{34,0,221},{37,0,218},{40,0,215},{43,0,212},{45,0,210},{48,0,207},{51,0,204},{53,0,202},{56,0,199},{59,0,196},{61,0,194},{64,0,191},{67,0,188},{69,0,186},{72,0,183},{75,0,180},{77,0,178},{80,0,175},{83,0,172},{85,0,171},{87,0,169},{90,0,166},{93,0,163},{95,0,161},{98,0,158},{101,0,155},{103,0,153},{106,0,150},{109,0,147},{111,0,145},{114,0,142},{117,0,139},{119,0,137},{122,0,134},{125,0,131},{128,0,128},{130,0,126},{133,0,123},{136,0,120},{138,0,118},{141,0,115},{144,0,112},{146,0,110},{149,0,107},{152,0,104},{154,0,102},{157,0,99},{160,0,96},{162,0,94},{165,0,91},{168,0,88},{170,0,85},{172,0,83},{175,0,80},{178,0,77},{180,0,75},{183,0,72},{186,0,69},{188,0,67},{191,0,64},{194,0,61},{196,0,59},{199,0,56},{202,0,53},{204,0,51},{207,0,48},{210,0,45},{213,0,42},{215,0,40},{218,0,37},{221,0,34},{223,0,32},{226,0,29},{229,0,26},{231,0,24},{234,0,21},{237,0,18},{239,0,16},{242,0,13},{245,0,10},{247,0,8},{250,0,5},{253,0,2}
};

extern float novelty_curve_normalized[NOVELTY_HISTORY_LENGTH];

void draw_sprite(float dest[], float sprite[], uint32_t dest_length, uint32_t sprite_length, float position, float alpha) {
  int32_t position_whole = position;  // Downcast to integer accuracy
  float position_fract = position - position_whole;
  float mix_right = position_fract;
  float mix_left = 1.0 - mix_right;

  for (uint16_t i = 0; i < sprite_length; i++) {
    int32_t pos_left = i + position_whole;
    int32_t pos_right = i + position_whole + 1;

    bool skip_left = false;
    bool skip_right = false;

    if (pos_left < 0) {
      pos_left = 0;
      skip_left = true;
    }
    if (pos_left > dest_length - 1) {
      pos_left = dest_length - 1;
      skip_left = true;
    }

    if (pos_right < 0) {
      pos_right = 0;
      skip_right = true;
    }
    if (pos_right > dest_length - 1) {
      pos_right = dest_length - 1;
      skip_right = true;
    }

    if (skip_left == false) {
      dest[pos_left] += sprite[i] * mix_left * alpha;
    }

    if (skip_right == false) {
      dest[pos_right] += sprite[i] * mix_right * alpha;
    }
  }
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
	//profile_function([&]() {
		float* ptr = (float*)input;
		dsps_mulc_f32_ae32(ptr + 0, ptr + 0, array_length, LUT.r, 3, 3);
		dsps_mulc_f32_ae32(ptr + 1, ptr + 1, array_length, LUT.g, 3, 3);
		dsps_mulc_f32_ae32(ptr + 2, ptr + 2, array_length, LUT.b, 3, 3);
	//}, __func__);

}

void scale_CRGBF_array_by_constant(CRGBF* input, float scale_value, uint16_t array_length) {
	//profile_function([&]() {
		float* ptr = (float*)input;
		dsps_mulc_f32_ae32(ptr, ptr, array_length * 3, scale_value, 1, 1);
	//}, __func__);
}

void add_CRGBF_arrays(CRGBF* a, CRGBF* b, uint16_t array_length) {
	//profile_function([&]() {
		float* ptr_a = (float*)a;
		float* ptr_b = (float*)b;

		dsps_add_f32(ptr_a, ptr_b, ptr_a, array_length * 3, 1, 1, 1);
	//}, __func__);
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

void fill_color(CRGBF* layer, uint16_t length, CRGBF color){
	for(uint16_t i = 0; i < length; i++){
		layer[i] = color;
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
	CRGBF return_val;
	profile_function([&]() {
		// Normalize hue to range [0, 1]
		h = fmodf(h, 1.0f);
		if (h < 0.0f) h += 1.0f;

		v = clip_float(v); // Ensure v is within the range [0.0, 1.0]
		float c = v * s; // Chroma
		float h_prime = h * 6.0f;
		float x = c * (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
		float m = v - c;

		float r = 0.0f, g = 0.0f, b = 0.0f;
		int sector = (int)h_prime;
		switch (sector) {
			case 0: r = c; g = x; break;
			case 1: r = x; g = c; break;
			case 2: g = c; b = x; break;
			case 3: g = x; b = c; break;
			case 4: r = x; b = c; break;
			case 5: r = c; b = x; break;
		}

		r += m; g += m; b += m;

		return_val = (CRGBF){r, g, b};
	}, __func__);

	return return_val;
}

void apply_blue_light_filter(float mix) {
	profile_function([&]() {
		if(mix > 0.0){
			multiply_CRGBF_array_by_LUT(
				leds,
				{
					float(incandescent_lookup.r * mix + (1.0 - mix)),
					float(incandescent_lookup.g * mix + (1.0 - mix)),
					float(incandescent_lookup.b * mix + (1.0 - mix))
				},
				NUM_LEDS
			);
		}
	}, __func__);

	/*
	float mix_inv = 1.0 - mix;
	save_leds_to_temp();
	multiply_CRGBF_array_by_LUT(leds_temp, incandescent_lookup, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds_temp, mix, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds, mix_inv, NUM_LEDS);
	add_CRGBF_arrays(leds, leds_temp, NUM_LEDS);
	*/
}

void save_leds_to_last() {
	memcpy(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);
}

CRGBF mix(CRGBF color_1, CRGBF color_2, float amount) {
	CRGBF out_color = { 0,0,0 };

	dsps_mulc_f32_ae32(&color_1.r, &color_1.r, 3, (1.0 - amount), 1, 1);
	dsps_mulc_f32_ae32(&color_2.r, &color_2.r, 3, (amount), 1, 1);
	dsps_add_f32(&color_1.r, &color_2.r, &out_color.r, 3, 1, 1, 1);

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

inline void ___(){
	// Either you:
	//   - accidentally found where this weird "___()" function is called from
	//   - are simply having fun reading 300 lines into leds.h because you're a big ol' geek
	//   - Ctrl-F'd the copyright notice to fuck with it or remove it for your Temu bootleg. Barf - get a real job, make your own products. I'm just trying to feed my family.

	static bool zz = false;
	if(zz == false && t_now_ms >= 10000){
		zz = true;

		printf("############################################################################\n");
		printf("                                  __  _\n");
		printf("            ___  ____ ___  ____  / /_(_)_____________  ____  ___\n");
		printf("           / _ \\/ __ `__ \\/ __ \\/ __/ / ___/ ___/ __ \\/ __ \\/ _ \\\n");
		printf("          /  __/ / / / / / /_/ / /_/ (__  ) /__/ /_/ / /_/ /  __/\n");
		printf("          \\___/_/ /_/ /_/\\____/\\__/_/____/\\___/\\____/ .___/\\___/\n");
		printf("              Audio-visual engine by @lixielabs    /_/\n");
		printf("              Released under the GPLv3 License\n");
		printf("############################################################################\n");
		printf("\n");
		printf("######################\n");
		printf("HARDWARE VERSION: %d\n", HARDWARE_VERSION);
		printf("SOFTWARE VERSION: %d.%d.%d\n", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH);
		printf("ESP-IDF  VERSION: %s\n", IDF_VER);
		printf("######################\n");
		printf("\n");
		printf("NOTE:\n");
		printf("If you're seeing this message on a product that\n");
		printf("isn't an Emotiscope you purchased from Lixie Labs,\n");
		printf("then it may have been a bootleg! Commerical forks\n");
		printf("of Emotiscope that I officially endorse will have\n");
		printf("a different message here, and be listed at:\n");
		printf("emotiscope.rocks/forks.html.\n");
		printf("\n");
		printf("If you built one yourself, that's awesome. I hope you're\n");
		printf("enjoying it as much as I enjoyed making it, and I still\n");
		printf("hope you'll consider supporting me in the future.\n");
		printf("\n");
		printf("- @lixielabs\n\n");
	}
}

// Function to draw a line with motion blur effect
void draw_line(CRGBF* layer, float x1, float x2, CRGBF color, float opacity) {
    bool lighten = !(color.r == 0 && color.g == 0 && color.b == 0);

    // Scale positions to pixel range
    x1 *= (float)(NUM_LEDS - 1);
    x2 *= (float)(NUM_LEDS - 1);

    // Swap if x2 is less than x1 to ensure x1 is always the start
    if (x1 > x2) {
        float temp = x1;
        x1 = x2;
        x2 = temp;
    }

    // Calculate integer indices and coverage for the start and end pixels
    int ix1 = (int)floor(x1);
    int ix2 = (int)ceil(x2);
    float start_coverage = 1.0 - (x1 - ix1);
    float end_coverage = x2 - floor(x2);

    // Loop through all affected pixels
    for (int i = ix1; i <= ix2; i++) {
        if (i >= 0 && i < NUM_LEDS) {
            float mix = opacity;
            if (i == ix1) mix *= start_coverage; // Adjust mix for start pixel
            else if (i == ix2) mix *= end_coverage; // Adjust mix for end pixel

			mix = sqrt(mix);

            if (lighten) {
                // Lighten mode: Add color
                layer[i].r += color.r * mix;
                layer[i].g += color.g * mix;
                layer[i].b += color.b * mix;
            } else {
                // Blend mode: Mix color
                layer[i].r = layer[i].r * (1.0 - mix) + color.r * mix;
                layer[i].g = layer[i].g * (1.0 - mix) + color.g * mix;
                layer[i].b = layer[i].b * (1.0 - mix) + color.b * mix;
            }
        }
    }
}

// Function to draw a dot with motion blur
void draw_dot(CRGBF* layer, uint16_t fx_dots_slot, CRGBF color, float position, float opacity = 1.0) {
    // Store previous position
    float prev_position = fx_dots[fx_dots_slot].position;
    fx_dots[fx_dots_slot].position = position;

    // Calculate distance moved and adjust brightness of spread accordingly
    float position_difference = fabs(position - prev_position);
    float spread_area = fmax(position_difference * NUM_LEDS, 1.0f);

    // Draw the line representing the motion blur
    draw_line(layer, prev_position, position, color, (1.0) * opacity);
}

void update_auto_color(){
	profile_function([&]() {
		if(light_modes[configuration.current_mode].type != LIGHT_MODE_TYPE_ACTIVE){ return; }

		static float color_momentum = 0.0;
		if(configuration.auto_color_cycle == true){
			float novelty = novelty_curve_normalized[NOVELTY_HISTORY_LENGTH - 1];
			novelty = novelty*novelty*novelty*novelty*novelty*novelty;

			color_momentum *= 0.95;
			color_momentum = fmax(color_momentum, novelty);
			if(color_momentum > 0.1){
				color_momentum = 0.1;
			}

			configuration.color += color_momentum*0.05;
		}
	}, __func__ );
}

void apply_phosphor_decay(float strength){
	static CRGBF phosphor_decay[NUM_LEDS];
	static bool first_run = true;

	if(first_run){
		first_run = false;
		memcpy(phosphor_decay, leds, sizeof(CRGBF) * NUM_LEDS);
		return;
	}

	strength = 1.0-strength;
	strength *= 0.05;

	strength = max(strength, 0.001f);

	//float strength_r = strength * incandescent_lookup.r;
	//float strength_g = strength * incandescent_lookup.g;
	//float strength_b = strength * incandescent_lookup.b;

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		float change_r = leds[i].r - phosphor_decay[i].r;
		float change_g = leds[i].g - phosphor_decay[i].g;
		float change_b = leds[i].b - phosphor_decay[i].b;

		if(change_r < -strength){ change_r = -strength; }
		if(change_g < -strength){ change_g = -strength; }
		if(change_b < -strength){ change_b = -strength; }

		leds[i].r = clip_float(phosphor_decay[i].r + change_r);
		leds[i].g = clip_float(phosphor_decay[i].g + change_g);
		leds[i].b = clip_float(phosphor_decay[i].b + change_b);
	}

	memcpy(phosphor_decay, leds, sizeof(CRGBF) * NUM_LEDS);
}

void render_debug_value() {
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
}

void apply_frame_blending(float blend_amount){
	static CRGBF previous_frame[NUM_LEDS];
	extern float lpf_drag;

	blend_amount = sqrt(sqrt( clip_float(fmax(blend_amount, sqrt(lpf_drag))) )) * 0.40 + 0.59;
	scale_CRGBF_array_by_constant(previous_frame, blend_amount, NUM_LEDS);

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		leds[i].r = max(leds[i].r, previous_frame[i].r);
		leds[i].g = max(leds[i].g, previous_frame[i].g);
		leds[i].b = max(leds[i].b, previous_frame[i].b);
	}

	memcpy(previous_frame, leds, sizeof(CRGBF) * NUM_LEDS);
}

void apply_box_blur(CRGBF* pixels, uint16_t num_pixels, int kernel_size) {
    // Ensure kernel size is odd for symmetry around the central pixel
    if (kernel_size % 2 == 0) {
        printf("Kernel size must be odd.\n");
        return;
    }

    int half_kernel = kernel_size / 2;

    // Temporary array to store blurred values
    CRGBF temp_pixels[num_pixels];
    memset(temp_pixels, 0, sizeof(temp_pixels));

    // Apply box blur to each pixel within the first 64 pixels
    for (int i = 0; i < num_pixels; ++i) {
        int valid_kernel_pixels = 0;
        CRGBF sum = {0.0f, 0.0f, 0.0f};

        // Sum the colors within the kernel's range, handle edges by duplicating pixels
        for (int k = -half_kernel; k <= half_kernel; ++k) {
            int pixel_index = i + k;

            // Handle OOB by duplicating edge pixels
            if (pixel_index < 0) pixel_index = 0;
            if (pixel_index >= num_pixels) pixel_index = num_pixels - 1;

            sum.r += pixels[pixel_index].r;
            sum.g += pixels[pixel_index].g;
            sum.b += pixels[pixel_index].b;

            valid_kernel_pixels++;
        }

        // Calculate the average and assign it to the temporary array
        temp_pixels[i].r = sum.r / valid_kernel_pixels;
        temp_pixels[i].g = sum.g / valid_kernel_pixels;
        temp_pixels[i].b = sum.b / valid_kernel_pixels;
    }

    // Copy the blurred values back to the original array
    memcpy(pixels, temp_pixels, sizeof(CRGBF) * num_pixels);
}

void apply_image_lpf(float cutoff_frequency) {
	profile_function([&]() {
		float alpha = 1.0 - expf(-6.28318530718 * cutoff_frequency / FPS_GPU);
		float alpha_inv = 1.0 - alpha;

		// Crasy fast SIMD-style math possible with the S3
		scale_CRGBF_array_by_constant(leds, alpha, NUM_LEDS);
		scale_CRGBF_array_by_constant(leds_last, alpha_inv, NUM_LEDS);

		add_CRGBF_arrays(leds, leds_last, NUM_LEDS);

		memcpy(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);
	}, __func__);
}

void apply_gamma_correction_to_color(CRGBF* color, float gamma) {
	// Create a new CRGBF object with gamma-corrected color components
	CRGBF corrected_color = {
		powf(color->r, gamma),  // Gamma correction for red component
		powf(color->g, gamma),  // Gamma correction for green component
		powf(color->b, gamma)   // Gamma correction for blue component
	};

	// Assign the gamma-corrected color back to the original CRGBF object
	*color = corrected_color;
}

void apply_gamma_correction() {
	profile_function([&]() {
		dsps_mul_f32_ae32((float*)leds, (float*)leds, (float*)leds, NUM_LEDS*3, 1, 1, 1);
	}, __func__);
}

void apply_brightness() {
	profile_function([&]() {
		float brightness_val = 0.3+configuration.brightness*0.7;

		scale_CRGBF_array_by_constant(leds, brightness_val, NUM_LEDS);
	}, __func__);
}

float get_color_range_hue(float progress){
	float return_val;
	profile_function([&]() {
		float color_range = configuration.color_range;
		
		if(color_range == 0.0){
			return_val = configuration.color;
		}	
		else if(configuration.reverse_color_range == true){
			color_range *= -1.0;
			return_val = (1.0-configuration.color) + (color_range * progress);
		}
		else{
			return_val = configuration.color + (color_range * progress);
		}
	}, __func__);

	return return_val;
}

void apply_background(float background_level){
	profile_function([&]() {
		background_level *= 0.25; // Max 25% brightness

		if(background_level > 0.0){
			if(configuration.mirror_mode == false){
				for(uint16_t i = 0; i < NUM_LEDS; i++){
					float progress = num_leds_float_lookup[i];
					CRGBF background_color = hsv(
						get_color_range_hue(progress),
						configuration.saturation,
						1.0
					);

					leds_temp[i] = background_color;
				}
			}
			else{
				for(uint16_t i = 0; i < (NUM_LEDS >> 1); i++){
					float progress = num_leds_float_lookup[i << 1];
					CRGBF background_color = hsv(
						get_color_range_hue(progress),
						configuration.saturation,
						1.0
					);
					
					int16_t left_index = 63-i;
					int16_t right_index = 64+i;

					leds_temp[left_index] = background_color;
					leds_temp[right_index] = background_color;
				}
			}

			// Apply background to the main buffer
			scale_CRGBF_array_by_constant(leds_temp,  background_level, NUM_LEDS);
			scale_CRGBF_array_by_constant(leds, 1.0 - background_level, NUM_LEDS);
			add_CRGBF_arrays(leds, leds_temp, NUM_LEDS);
		}
	}, __func__);
}

void clear_display(float keep = 0.0){
	profile_function([&]() {
		if(keep == 0.0){
			memset(leds, 0, sizeof(CRGBF)*NUM_LEDS);
		}
		else{
			scale_CRGBF_array_by_constant(leds, keep, NUM_LEDS);	
		}
	}, __func__);
}

void fade_display(){
	scale_CRGBF_array_by_constant(leds, configuration.softness, NUM_LEDS);
}