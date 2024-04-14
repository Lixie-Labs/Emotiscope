// UNCOMMENT THIS TO ENABLE IT
#define PROFILER_ENABLED  // Slows down the system, but allows watching how much
// total time each function takes up

#define PROFILER_SINGLE_SHOT \
	(true)	 // Count only the most recent run or all runs since last print?

#define PROFILER_HITS \
	(false)	 // Count the usage of a function instead of its execution time

#define PROFILER_PRINT_INTERVAL_MS \
	(5000)	// How long should data be gathered every period

extern void broadcast(char *message);
extern void print_websocket_clients(uint32_t t_now_ms);
extern char mac_str[18];

uint32_t t_now_ms = 0;
uint32_t t_now_us = 0;

profiler_function profiler_functions[128];
uint16_t num_profiled_functions = 0;
bool profiler_locked = false;

float FPS_CPU_SAMPLES[16];
float FPS_GPU_SAMPLES[16];

float FPS_CPU = 0.0;
float FPS_GPU = 0.0;

float CPU_CORE_USAGE = 0.0;

inline bool fastcmp_func_name(const char* input_a, const char* input_b){
	// Is first char different? DISQUALIFIED!
	if(input_a[0] != input_b[0]){ return false; }

	// If not, traditional strcmp(), return true for match
	return (strcmp(input_a, input_b) == 0);
}

template<typename MeasureFunc> // used for lambdas
float measure_execution(MeasureFunc func) {
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

int16_t find_matching_profiler_entry_index(const char *func_name) {
	for (uint16_t i = 0; i < num_profiled_functions; i++) {
		const char *profiled_name = profiler_functions[i].name;
		if(fastcmp_func_name(func_name, profiled_name)){
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

template<typename ProfileFunc> // used for lambdas
void profile_function(ProfileFunc func, const char* func_name) {
	#ifdef PROFILER_ENABLED
		int16_t index = find_matching_profiler_entry_index(func_name);
		// Not found yet? Register it:
		if (index == -1) {
			index = register_profiler_entry(func_name);
		}

		profiler_functions[index].hits += 1;
		uint32_t cycle_start = ESP.getCycleCount();

		// Run the function
		func();

		uint32_t cycle_end = ESP.getCycleCount();
		uint32_t num_cycles = cycle_end - cycle_start;

		if (PROFILER_SINGLE_SHOT == true) {
			profiler_functions[index].cycles_total = num_cycles;
		}
		else{
			profiler_functions[index].cycles_total += num_cycles;
		}
	#else
		// Just run the function
		func();
	#endif
}

void print_profiled_function_hits() {
	#ifdef PROFILER_ENABLED
	for (uint16_t i = 0; i < num_profiled_functions; i++) {
		if (PROFILER_HITS == true) {
			printf("HITS: %lu \t %s\n", profiler_functions[i].hits, profiler_functions[i].name);
		}
		else {
			uint32_t total_time = profiler_functions[i].cycles_total;
			printf("CYCLES: %lu \t %s\n", total_time, profiler_functions[i].name);
		}

		profiler_functions[i].cycles_total = 0;
		profiler_functions[i].hits = 0;
	}
	#endif
}

void watch_cpu_fps() {
	uint32_t us_now = micros();
	static uint32_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	uint32_t elapsed_us = us_now - last_call;
	FPS_CPU_SAMPLES[average_index % 16] = 1000000.0 / float(elapsed_us);
	last_call = us_now;
}

void watch_gpu_fps() {
	uint32_t us_now = micros();
	static uint32_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	uint32_t elapsed_us = us_now - last_call;
	FPS_GPU_SAMPLES[average_index % 16] = 1000000.0 / float(elapsed_us);

	last_call = us_now;
}

void print_system_info() {
	static uint32_t next_print_ms = 0;
	const uint16_t print_interval_ms = 5000;

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

	    uint32_t free_heap = esp_get_free_heap_size();
		UBaseType_t free_stack_cpu = uxTaskGetStackHighWaterMark(NULL); // CPU core (this one)
		UBaseType_t free_stack_gpu = uxTaskGetStackHighWaterMark(xTaskGetHandle("loop_gpu")); // GPU core

		extern volatile bool web_server_ready;
		extern PsychicWebSocketClient *get_client_in_slot(uint8_t slot);

		char stat_buffer[64] = { 0 };
		
		memset(stat_buffer, 0, 64);
		snprintf(stat_buffer, 64, "fps_cpu|%li", int16_t(FPS_CPU));
		broadcast(stat_buffer);

		memset(stat_buffer, 0, 64);
		snprintf(stat_buffer, 64, "fps_gpu|%li", int16_t(FPS_GPU));
		broadcast(stat_buffer);

		memset(stat_buffer, 0, 64);
		snprintf(stat_buffer, 64, "heap|%lu", (uint32_t)free_heap);
		broadcast(stat_buffer);

		printf("# SYSTEM INFO ####################\n");
		printf("CPU CORE USAGE --- %.2f%%\n", CPU_CORE_USAGE*100);
		printf("CPU FPS ---------- %.3f\n", FPS_CPU);
		printf("GPU FPS ---------- %.3f\n", FPS_GPU);
		printf("Free Heap -------- %lu\n", (uint32_t)free_heap);
		printf("Free Stack CPU --- %lu\n", (uint32_t)free_stack_cpu);
		printf("Free Stack GPU --- %lu\n", (uint32_t)free_stack_gpu);
		//printf("Total PSRAM ------ %lu\n", (uint32_t)ESP.getPsramSize());
		//printf("Free PSRAM ------- %lu\n", (uint32_t)ESP.getFreePsram());
		printf("IP Address ------- %s\n", WiFi.localIP().toString().c_str());
		printf("MAC Address ------ %s\n", mac_str);
		printf("\n");
		printf("- WS CLIENTS -----------------\n");
		if(web_server_ready == true){
			for(uint16_t i = 0; i < MAX_WEBSOCKET_CLIENTS; i++){
				PsychicWebSocketClient *client = get_client_in_slot(i);
				if (client != NULL) {
					printf("%s\n", client->remoteIP().toString().c_str());
				}
			}
		}
		printf("------------------------------\n");

		print_profiled_function_hits();	
		printf("##################################\n\n");
	}
}