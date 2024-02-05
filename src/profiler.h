// UNCOMMENT THIS TO ENABLE IT
//#define PROFILER_ENABLED  // Slows down the system, but allows watching how much
// total time each function takes up

#define PROFILER_SINGLE_SHOT \
	(true)	 // Count only the most recent run or all runs since last print?

#define PROFILER_HITS \
	(false)	 // Count the usage of a function instead of its execution time

#define PROFILER_PRINT_INTERVAL_MS \
	(5000)	// How long should data be gathered every period

extern void broadcast(char *message);
extern void print_websocket_clients(uint32_t t_now_ms);

Ticker profiler_print;

profiler_function profiler_functions[128];
uint16_t num_profiled_functions = 0;
bool profiler_locked = false;

float FPS_CPU_SAMPLES[16];
float FPS_GPU_SAMPLES[16];

float FPS_CPU = 0.0;
float FPS_GPU = 0.0;

template<typename Func> // used for lambdas
float measure_execution(Func func) {
	volatile uint32_t dummy = 1; // Volatile dummy variable to prevent loop optimization
    uint32_t t_start_us = ESP.getCycleCount();

	// Execute the lambda eight times to get an average
	func(); dummy += dummy;
	func(); dummy += dummy;
	func(); dummy += dummy;
	func(); dummy += dummy;
	func(); dummy += dummy;
	func(); dummy += dummy;
	func(); dummy += dummy;
	func(); dummy += dummy;

    uint32_t t_end_us = ESP.getCycleCount();
    uint32_t total_time_us = t_end_us - t_start_us;

	// Use the dummy variable in a way that does not affect the function's outcome
    // but prevents the compiler from optimizing the dummy operation away
    if (dummy == UINT32_MAX) {
        printf("This will never happen: %lu\n", dummy);
    }
    
    // Return the average execution time per iteration in microseconds as a float
    return (static_cast<float>(total_time_us) / 8) / 240; // divided 8 (averaging), then by (CPU MHz), to get sub-microsecond resolution
}

void print_profiled_function_hits() {
	printf("--------------------------------\n");
	printf("CPU: %f GPU: %f (FPS)\n", FPS_CPU, FPS_GPU);
	printf("FUNCS: %i\n\n", num_profiled_functions);

	for (uint16_t i = 0; i < num_profiled_functions; i++) {
		if (PROFILER_HITS == true) {
			printf("HITS: %lu \t %s\n", profiler_functions[i].hits, profiler_functions[i].name);
		}
		else {
			uint32_t total_time = profiler_functions[i].time_total;
			printf("TIME: %lu \t %s\n", total_time, profiler_functions[i].name);
		}

		profiler_functions[i].time_total = 0;
		profiler_functions[i].time_start = 0;
		profiler_functions[i].time_end = 0;
		profiler_functions[i].hits = 0;
	}
}

void init_profiler() {
#ifdef PROFILER_ENABLED
	profiler_locked = true;
	profiler_print.attach_ms(PROFILER_PRINT_INTERVAL_MS, print_profiled_function_hits);
	profiler_locked = false;
#endif
}

int16_t find_matching_profiler_entry_index(const char *func_name) {
	for (uint16_t i = 0; i < num_profiled_functions; i++) {
		const char *profiled_name = profiler_functions[i].name;
		int16_t match_val = strcmp(func_name, profiled_name);
		if (match_val == 0) {
			return i;
		}
	}

	return -1;
}

uint16_t register_profiler_entry(const char *func_name) {
	uint16_t dest_index = num_profiled_functions;
	memcpy(profiler_functions[dest_index].name, func_name, 32);

	num_profiled_functions += 1;
	return dest_index;
}

int16_t start_function_timing(const char *func_name) {
#ifdef PROFILER_ENABLED
	int16_t index = find_matching_profiler_entry_index(func_name);

	if (index == -1) {
		index = register_profiler_entry(func_name);
	}

	profiler_functions[index].hits += 1;
	profiler_functions[index].time_start = micros();
	return index;
#endif
	return 0;
}

void end_function_timing(uint16_t index) {
#ifdef PROFILER_ENABLED
	profiler_functions[index].time_end = micros();
	uint32_t duration = profiler_functions[index].time_end - profiler_functions[index].time_start;

	if (PROFILER_SINGLE_SHOT == true) {
		profiler_functions[index].time_total = duration;
	}
	else{
		profiler_functions[index].time_total += duration;
	}
#endif
}

void watch_cpu_fps(uint32_t t_now_us) {
	uint16_t profiler_index = start_function_timing(__func__);
	static uint32_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	uint32_t elapsed_us = t_now_us - last_call;
	FPS_CPU_SAMPLES[average_index % 16] = 1000000.0 / float(elapsed_us);
	last_call = t_now_us;

	end_function_timing(profiler_index);
}

void watch_gpu_fps(uint32_t t_now_us) {
	uint16_t profiler_index = start_function_timing(__func__);
	static uint32_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	uint32_t elapsed_us = t_now_us - last_call;
	FPS_GPU_SAMPLES[average_index % 16] = 1000000.0 / float(elapsed_us);

	last_call = t_now_us;

	end_function_timing(profiler_index);
}

void print_fps_values(uint32_t t_now_ms) {
	static uint32_t next_print_ms = 0;
	const uint16_t print_interval_ms = 1000;

	if (t_now_ms >= next_print_ms) {
		next_print_ms += print_interval_ms;

		FPS_CPU = 0.0;
		FPS_GPU = 0.0;
		for (uint8_t i = 0; i < 16; i++) {
			FPS_CPU += FPS_CPU_SAMPLES[i];
			FPS_GPU += FPS_GPU_SAMPLES[i];
		}
		FPS_CPU /= 16.0;
		FPS_GPU /= 16.0;

		char output[64];
		snprintf(output, 64, "FPS | CPU: %.2f | GPU: %.2f\n", FPS_CPU, FPS_GPU);
		printf(output);
		// printf("I love Julie and Sage!\n");
	}
}