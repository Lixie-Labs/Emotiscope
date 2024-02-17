// ------------------------------------------------------------
//          _     _   _   _   _     _                    _
//         | |   (_) | | (_) | |   (_)                  | |
//  _   _  | |_   _  | |  _  | |_   _    ___   ___      | |__
// | | | | | __| | | | | | | | __| | |  / _ \ / __|     | '_ \ 
// | |_| | | |_  | | | | | | | |_  | | |  __/ \__ \  _  | | | |
//  \__,_|  \__| |_| |_| |_|  \__| |_|  \___| |___/ (_) |_| |_|
//
// Custom math functions for things like array manipulation
// and system diagnostics like free stack/heap

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>

void broadcast(char* message){
	extern PsychicWebSocketHandler websocket_handler;
	websocket_handler.sendAll(message);
	//printf("%s\n", message);
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

float clip_float(float input) { return min(1.0f, max(0.0f, input)); }

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

// Function to perform autocorrelation using a given pattern of 128 samples within a larger data set
// Searches for the best correlation within 512 shift positions
// Returns the shift value where the pattern has the highest correlation
int autocorrelate_with_pattern(const float* data, unsigned int data_length, const float* pattern, unsigned int pattern_length) {
	// Check if the data array and pattern array are of sufficient length
	if (data_length < 128 || pattern_length != 128 || data_length < pattern_length + 512) {
		return -1;	// Error: Arrays not of correct length or data array not long enough for 512 shifts
	}

	unsigned int max_shifts = data_length - pattern_length;
	float correlations[max_shifts];
	for (unsigned int i = 0; i < max_shifts; i++) {
		correlations[i] = 0.0f;
		for (unsigned int j = 0; j < pattern_length; j++) {
			if (i + j < data_length) {
				correlations[i] += data[i + j] * pattern[j];
			}
		}
	}

	// Find the peak (maximum correlation) within the first 512 shifts
	int max_shift = 0;
	float max_correlation = correlations[0];
	for (unsigned int shift = 1; shift < 512 && shift < max_shifts; shift++) {
		if (correlations[shift] > max_correlation) {
			max_correlation = correlations[shift];
			max_shift = shift;
		}
	}

	return max_shift;
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

    // Temporary variable to hold intermediate filter results
    float filtered_value;

    // Apply the filter multiple times based on filter_order
    for (uint8_t order = 0; order < filter_order; ++order) {
        // Initialize the first filtered value
        filtered_value = input_array[0];

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