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

#define MAX_DOTS 384

CRGBF WHITE_BALANCE = { 1.0, 0.9375, 0.84 };

fx_dot fx_dots[MAX_DOTS];

CRGBF WHITE = {1.0, 1.0, 1.0};
CRGBF BLACK = {0.0, 0.0, 0.0};

CRGBF leds[NUM_LEDS]; // 32-bit image buffer
CRGBF leds_scaled[NUM_LEDS]; // scaled up to 8-bit range, but still floating point
CRGBF leds_temp[NUM_LEDS]; // for temporary copies of the image buffer (scaling)
CRGBF leds_last[NUM_LEDS];
CRGBF leds_smooth[NUM_LEDS];

float rendered_debug_value = 0.0;

CRGBF incandescent_lookup = {1.0000, 0.4452, 0.1562};

float note_colors[12] = {0.0000, 0.0833, 0.1666, 0.2499, 0.3333, 0.4166,
						 0.4999, 0.5833, 0.6666, 0.7499, 0.8333, 0.9166};

float color_momentum = 0.0;

float forced_frame_blending = 0.0;

extern float novelty_curve_normalized[NOVELTY_HISTORY_LENGTH];
extern light_mode light_modes[];

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
	start_profile(__COUNTER__, __func__);
	float* ptr = (float*)input;
	dsps_mulc_f32_ae32_fast(ptr + 0, ptr + 0, array_length, LUT.r, 3, 3);
	dsps_mulc_f32_ae32_fast(ptr + 1, ptr + 1, array_length, LUT.g, 3, 3);
	dsps_mulc_f32_ae32_fast(ptr + 2, ptr + 2, array_length, LUT.b, 3, 3);
	end_profile();
}

void add_CRGBF_arrays(CRGBF* a, CRGBF* b, uint16_t array_length) {
	float* ptr_a = (float*)a;
	float* ptr_b = (float*)b;
	dsps_add_f32_ae32_fast(ptr_a, ptr_b, ptr_a, array_length * 3, 1);
}


void scale_CRGBF_array_by_constant(CRGBF* input, float scale_value, uint16_t array_length) {
	float* ptr = (float*)input;
	dsps_mulc_f32_ae32_fast(ptr, ptr, array_length * 3, scale_value, 1, 1);
}


void init_fx_dots(){
	dsps_memset_aes3(fx_dots, 0.5, sizeof(fx_dot) * MAX_DOTS);
}

void clear_display(float keep){
	start_profile(__COUNTER__, __func__);
	if(keep == 0.0){
		dsps_memset_aes3(leds, 0, sizeof(CRGBF)*NUM_LEDS);
	}
	else{
		scale_CRGBF_array_by_constant(leds, keep, NUM_LEDS);	
	}
	end_profile();
}


CRGBF hsv(float h, float s, float v) {
	CRGBF return_val;
	// Normalize hue to range [0, 1]
	h = fmodf(h, 1.0f);
	if (h < 0.0f) h += 1.0f;

	//v = clip_float(v); // Ensure v is within the range [0.0, 1.0]
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

	return return_val;
}


float get_color_range_hue(float progress){
	if(configuration.color_mode.value.u32 == COLOR_MODE_PERLIN){
		progress = perlin_noise_array[(uint16_t)(progress * (NUM_LEDS>>2))];
	}

	float return_val;
	float color_range = configuration.color_range.value.f32;
	
	if(color_range == 0.0){
		return_val = configuration.color.value.f32;
	}	
	else if(configuration.reverse_color_range.value.u32 == true){
		color_range *= -1.0;
		return_val = (1.0-configuration.color.value.f32) + (color_range * progress);
	}
	else{
		return_val = configuration.color.value.f32 + (color_range * progress);
	}

	return return_val;
}


void fill_color(CRGBF* layer, uint16_t length, CRGBF color){
	for(uint16_t i = 0; i < length; i++){
		layer[i] = color;
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
                //layer[i].r = layer[i].r * (1.0 - mix) + color.r * mix;
                //layer[i].g = layer[i].g * (1.0 - mix) + color.g * mix;
                //layer[i].b = layer[i].b * (1.0 - mix) + color.b * mix;
            }
        }
    }
}


// Function to draw a dot with motion blur
void draw_dot(CRGBF* layer, uint16_t fx_dots_slot, CRGBF color, float position, float opacity) {
    // Store previous position
    float prev_position = fx_dots[fx_dots_slot].position;
    fx_dots[fx_dots_slot].position = position;

    // Calculate distance moved and adjust brightness of spread accordingly
    //float position_difference = fabs(position - prev_position);
    //float spread_area = fmaxf( (sqrt(position_difference)) * NUM_LEDS, 1.0f );
    // Draw the line representing the motion blur
    draw_line(layer, prev_position, position, color, (1.0) * opacity);
}


void apply_background(float background_level){
	start_profile(__COUNTER__, __func__);

	if(light_modes[configuration.current_mode.value.u32].type == LIGHT_MODE_TYPE_SYSTEM){
		end_profile();
		return;
	}

	background_level *= 0.25; // Max 25% brightness

	if(background_level > 0.0){
		if(configuration.mirror_mode.value.u32 == false){
			for(uint16_t i = 0; i < NUM_LEDS; i++){
				float progress = num_leds_float_lookup[i];
				CRGBF background_color = hsv(
					get_color_range_hue(progress),
					configuration.saturation.value.f32,
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
					configuration.saturation.value.f32,
					1.0
				);
				
				int16_t left_index  = ((NUM_LEDS>>1)-1) - i;
				int16_t right_index = ((NUM_LEDS>>1))   + i;

				leds_temp[left_index] = background_color;
				leds_temp[right_index] = background_color;
			}
		}

		// Apply background to the main buffer
		scale_CRGBF_array_by_constant(leds_temp,  background_level, NUM_LEDS);
		add_CRGBF_arrays(leds, leds_temp, NUM_LEDS);
	}

	end_profile();
}


void apply_box_blur( CRGBF* pixels, uint16_t num_pixels, int kernel_size ){
	dsps_memcpy_aes3( leds_temp, pixels, sizeof(CRGBF) * num_pixels );

	for( uint16_t i = 0; i < num_pixels; i++ ){
		int16_t kernel_far_left  = i - kernel_size;
		int16_t kernel_far_right = i + kernel_size;

		int16_t kernel_far_left_clipped  = MAX( (int16_t)0,                (int16_t)kernel_far_left  );
		int16_t kernel_far_right_clipped = MIN( (int16_t)( num_pixels-1 ), (int16_t)kernel_far_right );	

		CRGBF sum = { 0.0f, 0.0f, 0.0f };

		for(int16_t j = kernel_far_left; j <= kernel_far_right; j++){
			int16_t index = MAX( kernel_far_left_clipped, MIN( j, kernel_far_right_clipped ) );
			sum.r += leds_temp[ index ].r;
			sum.g += leds_temp[ index ].g;
			sum.b += leds_temp[ index ].b;
		}

		pixels[ i ] = sum;
	}

	scale_CRGBF_array_by_constant( pixels, 1.0 / ( kernel_size*2.0 + 1.0 ), num_pixels );
}


void apply_blur( float kernel_size ){
	start_profile(__COUNTER__, __func__);
	if(kernel_size < 0.01){
		end_profile();
		return;
	}
	
	apply_box_blur( leds, NUM_LEDS, kernel_size );

	if(kernel_size >= 2.0){
		apply_box_blur( leds, NUM_LEDS, kernel_size );
	}

	end_profile();
}


void apply_softness(){
	start_profile(__COUNTER__, __func__);
	extern float screensaver_mix;

	// This value decays itself non linearly toward zero all the time, 
	// *really* slowing down the LPF when it's set to 1.0.
	// This is a super hacky way to fake a true fade transition between modes
	forced_frame_blending *= 0.9975;

	if(forced_frame_blending < screensaver_mix*0.8){
		forced_frame_blending = screensaver_mix*0.8;
	}

	// Apply a low pass filter to every color channel of every pixel on every frame
	// at hundreds of frames per second
	// 
	// To anyone who reads this: microcontrollers are fucking insane now.
	// When I got into all this in 2012, I had a 16MHz single core AVR
	// 
	// The DMA and SIMD-style stuff inside the ESP32-S3 is some pretty crazy shit.
	float lpf_cutoff_frequency = 0.5 + (1.0-(sqrt(configuration.softness.value.f32)))*14.5;
	lpf_cutoff_frequency = lpf_cutoff_frequency * (1.0 - forced_frame_blending) + 0.5 * forced_frame_blending;
	
	float alpha = 1.0 - expf(-6.28318530718 * lpf_cutoff_frequency / FPS_GPU);
	float alpha_inv = 1.0 - alpha;

	// Crasy fast SIMD-style math possible with the S3
	scale_CRGBF_array_by_constant(leds, alpha, NUM_LEDS);
	scale_CRGBF_array_by_constant(leds_last, alpha_inv, NUM_LEDS);

	add_CRGBF_arrays(leds, leds_last, NUM_LEDS);

	dsps_memcpy_aes3(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);

	end_profile();
}


void apply_brightness() {
	start_profile(__COUNTER__, __func__);
	if(light_modes[configuration.current_mode.value.u32].type == LIGHT_MODE_TYPE_SYSTEM){
		end_profile();
		return;
	}

	float brightness_val = 0.3 + (configuration.brightness.value.f32*0.7);

	scale_CRGBF_array_by_constant(leds, brightness_val, NUM_LEDS);

	end_profile();
}


float soft_clip_hdr(float input) {
	if (input < 0.9) {
		// Linear function: output is the same as input for values less than 0.9
		return input;
	} else {
		// Non-linear function: transforms input values >= 0.9 to soft clipped values between 0.9 and 1.0
		float t = (input - 0.9) * 10.0;  // Scale input to enhance the soft clipping curve effect
		return 0.9 + 0.1 * tanh(t);    // Use hyperbolic tangent to provide a soft transition to 1.0
	}
}


void apply_tonemapping() {
	start_profile(__COUNTER__, __func__);
	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		leds[i].r = soft_clip_hdr(leds[i].r);
		leds[i].g = soft_clip_hdr(leds[i].g);
		leds[i].b = soft_clip_hdr(leds[i].b);
	}
	end_profile();
}


void apply_warmth(float mix) {
	start_profile(__COUNTER__, __func__);
	if(light_modes[configuration.current_mode.value.u32].type == LIGHT_MODE_TYPE_SYSTEM){
		end_profile();
		return;
	}

	float mix_inv = 1.0 - mix;
	if(mix > 0.0){
		multiply_CRGBF_array_by_LUT(
			leds,
			(CRGBF){
				incandescent_lookup.r * mix + mix_inv,
				incandescent_lookup.g * mix + mix_inv,
				incandescent_lookup.b * mix + mix_inv
			},
			NUM_LEDS
		);
	}

	end_profile();
}


void apply_master_brightness(){
	start_profile(__COUNTER__, __func__);
	static float master_brightness = 0.0;
	if(t_now_ms >= 1000){
		if(master_brightness < 1.0){
			master_brightness += 0.001;
		}
	}

	scale_CRGBF_array_by_constant(leds, clip_float(master_brightness), NUM_LEDS);
	end_profile();
}


void apply_gamma_correction() {
	start_profile(__COUNTER__, __func__);
	static bool first_run = true;
	if(first_run == true){
		first_run = false;

		// Generate a lookup table for gamma correction
		//printf("gamma_correction_lookup[2048] = {");

		for(uint16_t i = 0; i < 2048; i++){
			float gamma = 1.5;
			float corrected = powf(
				(float)i / 2047.0,
				gamma
			);
			
			//printf("%.4f, ", corrected);
			gamma_correction_lookup[i] = corrected;
		}

		//printf("};\n");
	}

	for(uint16_t i = 0; i < NUM_LEDS; i++){
		leds[i].r = gamma_correction_lookup[(uint16_t)(leds[i].r * 2047)]; 
		leds[i].g = gamma_correction_lookup[(uint16_t)(leds[i].g * 2047)];
		leds[i].b = gamma_correction_lookup[(uint16_t)(leds[i].b * 2047)];
	}

	end_profile();
}


CRGBF add(CRGBF color_1, CRGBF color_2) {
	CRGBF out_color = {
		color_1.r + color_2.r,
		color_1.g + color_2.g,
		color_1.b + color_2.b,
	};

	return out_color;
}


void draw_sprite(CRGBF dest[], CRGBF sprite[], uint16_t dest_length, uint16_t sprite_length, float position, float alpha){
	int16_t position_whole = floor(position);  // Downcast to integer accuracy
	float position_fract = fabsf(fabsf(position) - fabsf(position_whole));

	for (int16_t i = 0; i < sprite_length; i++) {
		int16_t pos_left = i + position_whole;
		int16_t pos_right = i + position_whole + 1;

		float mix_right = position_fract;
		float mix_left = 1.0 - mix_right; 

		if (pos_left >= 0 && pos_left < dest_length) {
			dest[pos_left].r += sprite[i].r * mix_left * alpha;
			dest[pos_left].g += sprite[i].g * mix_left * alpha;
			dest[pos_left].b += sprite[i].b * mix_left * alpha;
		}

		if (pos_right >= 0 && pos_right < dest_length) {
			dest[pos_right].r += sprite[i].r * mix_right * alpha;
			dest[pos_right].g += sprite[i].g * mix_right * alpha;
			dest[pos_right].b += sprite[i].b * mix_right * alpha;
		}
	}
}

void draw_sprite_float(float dest[], float sprite[], uint32_t dest_length, uint32_t sprite_length, float position, float alpha) {
	int16_t position_whole = floor(position);  // Downcast to integer accuracy
	float position_fract = fabsf(fabsf(position) - fabsf(position_whole));

	for (int16_t i = 0; i < sprite_length; i++) {
		int16_t pos_left = i + position_whole;
		int16_t pos_right = i + position_whole + 1;

		float mix_right = position_fract;
		float mix_left = 1.0 - mix_right;

		if (pos_left >= 0 && pos_left < dest_length) {
			dest[pos_left] += sprite[i] * mix_left * alpha;
		}

		if (pos_right >= 0 && pos_right < dest_length) {
			dest[pos_right] += sprite[i] * mix_right * alpha;
		}
	}
}


void update_auto_color(){
	start_profile(__COUNTER__, __func__);
	if(light_modes[configuration.current_mode.value.u32].type != LIGHT_MODE_TYPE_ACTIVE){
		end_profile();
		return;
	}

	if(configuration.auto_color_cycle.value.u32 == true){
		float novelty = novelty_curve_normalized[NOVELTY_HISTORY_LENGTH - 1] * 0.75;
		novelty = novelty*novelty*novelty*novelty*novelty*novelty;

		color_momentum *= 0.95;
		color_momentum = fmaxf(color_momentum, novelty);
		if(color_momentum > 0.1){
			color_momentum = 0.1;
		}

		configuration.color.value.f32 += color_momentum*0.05;
	}

	end_profile();
}

void clamp_configuration(){
	configuration.color.value.f32 = fmodf(configuration.color.value.f32, 1.0);
}

/*



void save_leds_to_temp() {
	// uint16_t profiler_index = start_function_timing(__func__);
	dsps_memcpy_aes3(leds_temp, leds, sizeof(CRGBF) * NUM_LEDS);
	// end_function_timing(profiler_index);
}

void load_leds_from_temp() {
	// uint16_t profiler_index = start_function_timing(__func__);
	dsps_memcpy_aes3(leds, leds_temp, sizeof(CRGBF) * NUM_LEDS);
	// end_function_timing(profiler_index);
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
		dsps_memcpy_aes3(leds_smooth, leds, sizeof(CRGBF) * NUM_LEDS);
	}
}



void clip_leds() {
	// Loop unroll for speed
	for (uint16_t i = 0; i < NUM_LEDS; i += 2) {
		leds[i + 0].r = clip_float(leds[i + 0].r);
		leds[i + 0].g = clip_float(leds[i + 0].g);
		leds[i + 0].b = clip_float(leds[i + 0].b);

		leds[i + 1].r = clip_float(leds[i + 1].r);
		leds[i + 1].g = clip_float(leds[i + 1].g);
		leds[i + 1].b = clip_float(leds[i + 1].b);
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



void save_leds_to_last() {
	dsps_memcpy_aes3(leds_last, leds, sizeof(CRGBF) * NUM_LEDS);
}

CRGBF mix(CRGBF color_1, CRGBF color_2, float amount) {
	CRGBF out_color = { 0,0,0 };

	dsps_mulc_f32_ae32(&color_1.r, &color_1.r, 3, (1.0 - amount), 1, 1);
	dsps_mulc_f32_ae32(&color_2.r, &color_2.r, 3, (amount), 1, 1);
	dsps_add_f32(&color_1.r, &color_2.r, &out_color.r, 3, 1, 1, 1);

	return out_color;
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







void apply_phosphor_decay(float strength){
	static CRGBF phosphor_decay[NUM_LEDS];
	static bool first_run = true;

	if(first_run){
		first_run = false;
		dsps_memcpy_aes3(phosphor_decay, leds, sizeof(CRGBF) * NUM_LEDS);
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

		leds[i].r = (phosphor_decay[i].r + change_r);
		leds[i].g = (phosphor_decay[i].g + change_g);
		leds[i].b = (phosphor_decay[i].b + change_b);
	}

	dsps_memcpy_aes3(phosphor_decay, leds, sizeof(CRGBF) * NUM_LEDS);
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

	blend_amount = sqrt(sqrt( clip_float(fmaxf(blend_amount, sqrt(lpf_drag))) )) * 0.40 + 0.59;
	scale_CRGBF_array_by_constant(leds, 1.0-blend_amount, NUM_LEDS);
	scale_CRGBF_array_by_constant(previous_frame, blend_amount, NUM_LEDS);
	add_CRGBF_arrays(leds, previous_frame, NUM_LEDS);

	dsps_memcpy_aes3(previous_frame, leds, sizeof(CRGBF) * NUM_LEDS);
}

void apply_fractional_blur(CRGBF* pixels, uint16_t num_pixels, float kernel_size) {
	if(kernel_size >= 0.001){
		// Ensure kernel_size is positive
		if (kernel_size <= 0) {
			printf("Kernel size must be positive.\n");
			return;
		}

		// Calculate the effective range of influence for the kernel size
		int range = (int)ceil(kernel_size * 3); // Typically 3 standard deviations are considered
		CRGBF temp_pixels[num_pixels];
		dsps_memset_aes3(temp_pixels, 0, sizeof(temp_pixels));

		for (int i = 0; i < num_pixels; ++i) {
			float total_weight = 0;
			CRGBF weighted_sum = {0.0f, 0.0f, 0.0f};

			// Apply weights to the pixels in the range
			for (int k = -range; k <= range; ++k) {
				int pixel_index = i + k;
				if (pixel_index < 0) pixel_index = 0;
				if (pixel_index >= num_pixels) pixel_index = num_pixels - 1;

				// Calculate the weight using a Gaussian-like function
				float distance = fabs(k);
				float weight = exp(-0.5 * (distance / kernel_size) * (distance / kernel_size));

				weighted_sum.r += pixels[pixel_index].r * weight;
				weighted_sum.g += pixels[pixel_index].g * weight;
				weighted_sum.b += pixels[pixel_index].b * weight;

				total_weight += weight;
			}

			// Normalize the weighted sum by the total weight
			temp_pixels[i].r = weighted_sum.r / total_weight;
			temp_pixels[i].g = weighted_sum.g / total_weight;
			temp_pixels[i].b = weighted_sum.b / total_weight;
		}

		// Copy the blurred values back to the original array
		dsps_memcpy_aes3(pixels, temp_pixels, sizeof(CRGBF) * num_pixels);
	}
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









void fade_display(){
	scale_CRGBF_array_by_constant(leds, configuration.softness.value.f32, NUM_LEDS);
}



void scramble_image( float distance ){
	dsps_memset_aes3(leds_temp, 0, sizeof(CRGBF) * NUM_LEDS);

	// Scramble the image
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		// Use ESP32-S3 hardware RNG to get a float between 0.0 and 1.0
		float random_value = get_random_float();

		float splat_position = (random_value - 0.5) * 2.0;
		float splat_opacity = 1.0 - fabs(splat_position);

		splat_position *= distance;

		// Calculate the new index for the pixel (signed integer)
		int16_t new_index = i + (int16_t)(splat_position);

		if(new_index > 0 && new_index < NUM_LEDS){
			// Copy the pixel to the new index
			leds_temp[new_index] = add(leds_temp[new_index], leds[i], splat_opacity);
		}
	}

	dsps_memcpy_aes3(leds, leds_temp, sizeof(CRGBF) * NUM_LEDS);
}



*/