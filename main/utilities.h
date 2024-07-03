/*
------------------------------------------------------------
         _     _   _   _   _     _                    _
        | |   (_) | | (_) | |   (_)                  | |
 _   _  | |_   _  | |  _  | |_   _    ___   ___      | |__
| | | | | __| | | | | | | | __| | |  / _ \ / __|     | '_ \ 
| |_| | | |_  | | | | | | | |_  | | |  __/ \__ \  _  | | | |
 \__,_|  \__| |_| |_| |_|  \__| |_|  \___| |___/ (_) |_| |_|

Custom math functions for things like array manipulation
and system diagnostics like free stack/heap
*/

#define PI	    3.14159265
#define TWOPI   6.28318530
#define FOURPI 12.56637061
#define SIXPI  18.84955593

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#define TOTAL_NOISE_SAMPLES 666 // and the devil laughs
uint8_t noise_samples[TOTAL_NOISE_SAMPLES];

float num_leds_float_lookup[NUM_LEDS];
float num_freqs_float_lookup[NUM_FREQS];
float num_tempi_float_lookup[NUM_TEMPI];
float random_float_lookup[1024];

char substring[128];

void shift_and_copy_arrays(float history_array[], size_t history_size, const float new_array[], size_t new_size) {
	// Use memmove to shift the history array
	memmove(history_array, history_array + new_size,
			(history_size - new_size) * sizeof(float));

	// Use dsps_memcpy_aes3 to copy the new array into the vacant space
	dsps_memcpy_aes3(history_array + history_size - new_size, new_array, new_size * sizeof(float));
}

float clip_float(float input) {
	return fminf(1.0f, fmaxf(0.0f, input));
}

void init_num_leds_float_lookup(){
	for(uint16_t i = 0; i < NUM_LEDS; i++){
		num_leds_float_lookup[i] = i / (float)NUM_LEDS;
	}
}

void init_num_freqs_float_lookup(){
	for(uint16_t i = 0; i < NUM_FREQS; i++){
		num_freqs_float_lookup[i] = i / (float)NUM_FREQS;
	}
}

void init_num_tempi_float_lookup(){
	for(uint16_t i = 0; i < NUM_TEMPI; i++){
		num_tempi_float_lookup[i] = i / (float)NUM_TEMPI;
	}
}

void init_random_float_lookup(){
	for(uint16_t i = 0; i < 1024; i++){
		random_float_lookup[i] = esp_random() / (float)UINT32_MAX;
	}
}

void init_floating_point_lookups(){
	init_num_leds_float_lookup();
	init_num_freqs_float_lookup();
	init_num_tempi_float_lookup();
	init_random_float_lookup();
}

void init_noise_samples(){
	for(uint16_t i = 0; i < TOTAL_NOISE_SAMPLES; i++){
		noise_samples[i] = esp_random() / (float)UINT32_MAX;
	}
}

float get_random_float(){
	static float position = 0.0;

	uint32_t index = (uint32_t)position * 1023;
	float push = random_float_lookup[index];
	if(push > 0.5){
		push += push*0.1;
	}

	position += (push*0.01);

	while(position >= 1.0){
		position -= 1.0;
	}

	return push;
}

void fetch_substring(char* input_buffer, char delimiter, uint8_t fetch_index){
	dsps_memset_aes3(substring, 0, 128);
	int16_t input_length = strlen(input_buffer);

	for(uint16_t i = 0; i < input_length; i++){
		if(fetch_index == 0){
			for(uint16_t j = i; j <= input_length; j++){
				if(input_buffer[j] == delimiter || input_buffer[j] == '\0'){
					dsps_memcpy_aes3(substring, input_buffer + i, j-i);
					return;
				}
			}
			return;
		}

		if(input_buffer[i] == delimiter){
			fetch_index--;
		}
	}
}

bool get_random_bit(){
	static uint16_t position = 0;
	position++;

	if(position >= TOTAL_NOISE_SAMPLES){
		position = 0;
	}

	return noise_samples[position];
}

// Can return a value between two array indices with linear interpolation
float interpolate(float index, float* array, uint16_t array_size) {
	float index_f = index * (array_size - 1);
	uint16_t index_i = (uint16_t)index_f;
	float index_f_frac = index_f - index_i;

	float left_val = array[index_i];
	float right_val = array[index_i + 1];

	if (index_i + 1 >= array_size) {
		right_val = left_val;
	}

	return (1 - index_f_frac) * left_val + index_f_frac * right_val;
}

// Function to shift array contents to the left
void shift_array_left(float* array, uint16_t array_size, uint16_t shift_amount) {
	// Check if the shift amount is greater than the array size
	if (shift_amount >= array_size) {
		// If yes, set the whole array to zero
		dsps_memset_aes3(array, 0, array_size * sizeof(float));
	}
	else {
		// Use memmove to shift array contents to the left
		memmove(array, array + shift_amount, (array_size - shift_amount) * sizeof(float));

		// Set the vacated positions at the end of the array to zero
		dsps_memset_aes3(array + array_size - shift_amount, 0, shift_amount * sizeof(float));
	}
}

bool fastcmp(const char* input_a, const char* input_b){
	// Is first char different? DISQUALIFIED!
	if(input_a[0] != input_b[0]){ return false; }

	// If not, traditional strcmp(), return true for match
	return (strcmp(input_a, input_b) == 0);
}

void print_binary(uint32_t input, uint8_t bit_width, char tail){
	char output[64];
	dsps_memset_aes3(output, 0, 64);
	for(uint8_t i = 0; i < bit_width; i++){
		output[i] = (char)('0' + bitRead(input, (bit_width-1)-i));
	}

	printf("%s%c", output, tail);
}

void low_pass_filter(float* input_array, uint16_t num_samples, uint16_t sample_rate, float cutoff_frequency, uint8_t filter_order) {
    // Calculate the smoothing factor alpha inline
    float rc = 1.0f / (2.0f * M_PI * cutoff_frequency);
    float alpha = 1.0f / (1.0f + (sample_rate * rc));

    // Apply the filter multiple times based on filter_order
    for (uint8_t order = 0; order < filter_order; ++order) {
        // Initialize the first filtered value
        float filtered_value = input_array[0];

        // Start filtering from the second sample
        for (uint16_t n = 1; n < num_samples; ++n) {
            // Implement the filter equation: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
            filtered_value = alpha * input_array[n] + (1.0f - alpha) * filtered_value;

            // Store the filtered result back into the array
            input_array[n] = filtered_value;
        }
    }
}

float fixed_interpolate(uint8_t value_a, uint8_t value_b, uint8_t factor) {
    // Precompute the inverse factor to save one subtraction operation later
    uint8_t inverse_factor = 255 - factor; // Use 255 to represent 1.0 in fixed-point for factor

    // Compute the weighted sum using only integer operations
    // The use of uint16_t for intermediate results ensures no overflow for 255*255
    uint16_t weighted_sum = value_a * inverse_factor + value_b * factor;

    // Convert to float and scale back by dividing by 255*255, which is the maximum possible value
    // Since we're dividing by a constant, this can be optimized by the compiler
    return weighted_sum / 65025.0f; // 255*255 = 65025
}

float fast_tanh(float x) {
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}