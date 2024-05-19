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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>

#define TOTAL_NOISE_SAMPLES 666 // and the devil laughs
uint8_t noise_samples[TOTAL_NOISE_SAMPLES];

float num_leds_float_lookup[NUM_LEDS];
float num_freqs_float_lookup[NUM_FREQS];
float num_tempi_float_lookup[NUM_TEMPI];

char substring[128];

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

void init_floating_point_lookups(){
	init_num_leds_float_lookup();
	init_num_freqs_float_lookup();
	init_num_tempi_float_lookup();
}

void init_noise_samples(){
	for(uint16_t i = 0; i < TOTAL_NOISE_SAMPLES; i++){
		noise_samples[i] = esp_random() & 1;
	}
}

void fetch_substring(char* input_buffer, char delimiter, uint8_t fetch_index){
	memset(substring, 0, 128);
	int16_t input_length = strlen(input_buffer);

	for(uint16_t i = 0; i < input_length; i++){
		if(fetch_index == 0){
			for(uint16_t j = i; j <= input_length; j++){
				if(input_buffer[j] == delimiter || input_buffer[j] == '\0'){
					memcpy(substring, input_buffer + i, j-i);
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

void broadcast(const char* message){
	extern PsychicWebSocketHandler websocket_handler;
	printf("TX: %s\n", message);
	websocket_handler.sendAll(message);
}

float linear_to_tri(float input) {
    if (input < 0.0f || input > 1.0f) {
        // Input out of range
        return -1.0f; // Return an error value (or handle as you see fit)
    }

    if (input <= 0.5f) {
        // Scale up the first half
        return 2.0f * input;
    } else {
        // Scale down the second half
        return 2.0f * (1.0f - input);
    }
}

inline bool get_random_bit(){
	static uint16_t position = 0;
	position++;

	if(position >= TOTAL_NOISE_SAMPLES){
		position = 0;
	}

	return noise_samples[position];
}

// Can return a value between two array indices with linear interpolation
float IRAM_ATTR interpolate(float index, float* array, uint16_t array_size) {
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

void shift_and_copy_arrays(float history_array[], size_t history_size, const float new_array[], size_t new_size) {
	profile_function([&]() {
		// Use memmove to shift the history array
		memmove(history_array, history_array + new_size,
				(history_size - new_size) * sizeof(float));

		// Use memcpy to copy the new array into the vacant space
		memcpy(history_array + history_size - new_size, new_array, new_size * sizeof(float));
	}, __func__ );
}

// Function to shift array contents to the left
void shift_array_left(float* array, uint16_t array_size, uint16_t shift_amount) {
	// Check if the shift amount is greater than the array size
	if (shift_amount >= array_size) {
		// If yes, set the whole array to zero
		memset(array, 0, array_size * sizeof(float));
	}
	else {
		// Use memmove to shift array contents to the left
		memmove(array, array + shift_amount, (array_size - shift_amount) * sizeof(float));

		// Set the vacated positions at the end of the array to zero
		memset(array + array_size - shift_amount, 0, shift_amount * sizeof(float));
	}
}

inline float clip_float(float input) { return min(1.0f, max(0.0f, input)); }

// Fast approximation of the square root using Newton-Raphson method
float fast_sqrt(float number) {
	// Initial guess for the square root
	float x = number / 2.0;
	// Tolerance for the accuracy of the result
	const float tolerance = 0.01;

	// Iterate to improve the estimate
	while (1) {
		float x_old = x;
		x = (x + number / x) / 2.0;
		// Check if the improvement is within the tolerance
		if (fabs(x - x_old) < tolerance) {
			break;
		}
	}

	return x;
}

inline bool fastcmp(char* input_a, char* input_b){
	// Is first char different? DISQUALIFIED!
	if(input_a[0] != input_b[0]){ return false; }

	// If not, traditional strcmp(), return true for match
	return (strcmp(input_a, input_b) == 0);
}

inline bool fastcmp(const char* input_a, const char* input_b){
	// Is first char different? DISQUALIFIED!
	if(input_a[0] != input_b[0]){ return false; }

	// If not, traditional strcmp(), return true for match
	return (strcmp(input_a, input_b) == 0);
}

inline bool fastcmp(char* input_a, const char* input_b){
	// Is first char different? DISQUALIFIED!
	if(input_a[0] != input_b[0]){ return false; }

	// If not, traditional strcmp(), return true for match
	return (strcmp(input_a, input_b) == 0);
}

void print_binary(uint32_t input, uint8_t bit_width, char tail){
	char output[64];
	memset(output, 0, 64);
	for(uint8_t i = 0; i < bit_width; i++){
		output[i] = char('0' + bitRead(input, (bit_width-1)-i));
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