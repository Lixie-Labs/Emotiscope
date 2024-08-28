//#define PROFILER_ENABLED true // Uncomment to enable, comment to disable

#ifdef __OPTIMIZE__
    #define OPT_LEVEL 2
#else
    #define OPT_LEVEL 0
#endif

int64_t t_now_ms = 0;
int64_t t_now_us = 0;

float FPS_CPU_SAMPLES[64];
float FPS_GPU_SAMPLES[64];

float CPU_CORE_USAGE = 0.0;
float FPS_CPU = 0.0;
float FPS_GPU = 0.0;
float CPU_TEMP = 0.0;
uint32_t FREE_HEAP = 0;

extern light_mode light_modes[];
extern config configuration;

__attribute__((aligned(16)))
int32_t function_stack[2][32];
uint8_t function_stack_pointer[2] = { 0, 0 };

profiled_function profiled_functions[128];

void check_optimization_level() {
	ESP_LOGI(TAG, "Optimization level: %u", OPT_LEVEL);
	if (OPT_LEVEL == 0) {
		ESP_LOGE(TAG, "!--- WARNING: Optimization level is set to 0, Emotiscope running slower than normal!");
	}
}

void init_profiler(){
	memset(function_stack, -1, sizeof(function_stack));
	memset(profiled_functions, 0, sizeof(profiled_function)*128);

	check_optimization_level();
}

void print_function_profile() {
	ESP_LOGI(TAG, "System Info ---------------------------");
	ESP_LOGI(TAG, "IP Address: %s", ip_str);
	ESP_LOGI(TAG, "Function Profile ----------------------");
	
	uint16_t num_profiled_functions = 0;
	for (uint8_t i = 0; i < 128; i++) {
		if (profiled_functions[i].name[0] != 0) {
			num_profiled_functions++;
		}
	}
	ESP_LOGI(TAG, "Profiled functions: %u\n", num_profiled_functions);

	ESP_LOGI(TAG, "CPU Core ############################");
	uint32_t max_cpu_hits = 0;
	for (uint8_t i = 0; i < 128; i++) {
		max_cpu_hits = MAX(profiled_functions[i].hits[1], max_cpu_hits);
	}
	for (uint8_t i = 0; i < 128; i++) {
		if (profiled_functions[i].hits[1] > 0) {
			ESP_LOGI(TAG, "%s: %.2f%% %lu", profiled_functions[i].name, (profiled_functions[i].hits[1]/(float)max_cpu_hits)*100.0, profiled_functions[i].hits[1]);
		}
	}

	ESP_LOGI(TAG, "GPU Core ############################");
	uint32_t max_gpu_hits = 0;
	for (uint8_t i = 0; i < 128; i++) {
		max_gpu_hits = MAX(profiled_functions[i].hits[0], max_gpu_hits);
	}
	for (uint8_t i = 0; i < 128; i++) {
		if (profiled_functions[i].hits[0] > 0) {
			ESP_LOGI(TAG, "%s: %.2f%% %lu", profiled_functions[i].name, (profiled_functions[i].hits[0]/(float)max_gpu_hits)*100.0, profiled_functions[i].hits[0]);
		}
	}

	for(uint8_t i = 0; i < 128; i++){
		profiled_functions[i].hits[0] = 0;
		profiled_functions[i].hits[1] = 0;
	}

	ESP_LOGI(TAG, "----------------------------------------");
}

inline void log_function_stack(){
	for(uint8_t i = 0; i < 32; i++){
		if(function_stack[0][i] != -1){ profiled_functions[function_stack[0][i]].hits[0]++; }
		else{ break; }
	}

	for(uint8_t i = 0; i < 32; i++){
		if(function_stack[1][i] != -1){ profiled_functions[function_stack[1][i]].hits[1]++; }
		else{ break; }
	}
}

#ifdef PROFILER_ENABLED
	void start_profile(uint32_t id, const char* name) {
		int core_number = xPortGetCoreID();
		function_stack[core_number][function_stack_pointer[core_number]] = id;
		function_stack_pointer[core_number]++;
		function_stack_pointer[core_number] = MIN(32-1, function_stack_pointer[core_number]);

		if (profiled_functions[id].name[0] == 0) {
			dsps_memcpy_aes3(profiled_functions[id].name, name, MIN(15, strlen(name)));
		}
	}

	void end_profile() {
		int core_number = xPortGetCoreID();
		function_stack_pointer[core_number]--;
		function_stack_pointer[core_number] = MAX(0, function_stack_pointer[core_number]);

		function_stack[core_number][function_stack_pointer[core_number]] = -1;
	}
#else
	inline void start_profile(uint32_t id, const char* name) {}
	inline void end_profile() {}
#endif

void watch_cpu_fps() {
	start_profile(__COUNTER__, __func__);
	int64_t us_now = esp_timer_get_time();	
	static int64_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	int64_t elapsed_us = us_now - last_call;
	FPS_CPU_SAMPLES[average_index % 64] = 1000000.0 / (float)elapsed_us;
	last_call = us_now;
	end_profile();
}

void watch_gpu_fps() {
	start_profile(__COUNTER__, __func__);
	int64_t us_now = esp_timer_get_time();
	static int64_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	int64_t elapsed_us = us_now - last_call;
	FPS_GPU_SAMPLES[average_index % 64] = 1000000.0 / (float)elapsed_us;

	last_call = us_now;
	end_profile();
}

void print_profiler_stats() {
	start_profile(__COUNTER__, __func__);

	static int64_t last_stat_print = 0;
	if (t_now_ms - last_stat_print >= 1000) {
		last_stat_print = t_now_ms;

		//ESP_LOGI(TAG, "CPU FPS: %.2f, GPU FPS: %.2f, CPU Usage: %.2f%% CPU Temp: %.2f, Free Heap: %lu, current_mode: %s", FPS_CPU, FPS_GPU, CPU_CORE_USAGE*100, CPU_TEMP, FREE_HEAP, light_modes[configuration.current_mode.value.u32].name);
	}

	#ifdef PROFILER_ENABLED
		if(t_now_ms > 5000){
			static int64_t last_profile_print = 0;
			if (t_now_ms - last_profile_print >= 10000) {
				last_profile_print = t_now_ms;
				print_function_profile();
			}
		}
	#endif

	end_profile();
}

void update_stats() {
	start_profile(__COUNTER__, __func__);
	const uint16_t update_hz = 10;
	const uint32_t update_interval = 1000 / update_hz;
	static int64_t last_update = 0;

	if (t_now_ms - last_update < update_interval) {
		end_profile();
		return;
	}

	FPS_CPU = 0.0;
	FPS_GPU = 0.0;
	for (uint8_t i = 0; i < 64; i++) {
		FPS_CPU += FPS_CPU_SAMPLES[i];
		FPS_GPU += FPS_GPU_SAMPLES[i];
	}
	FPS_CPU /= 64.0;
	FPS_GPU /= 64.0;

	CPU_TEMP = get_cpu_temperature();

	FREE_HEAP = heap_caps_get_largest_free_block(MALLOC_CAP_32BIT);

	//UBaseType_t free_stack_cpu = uxTaskGetStackHighWaterMark(NULL); // CPU core (this one)
	//UBaseType_t free_stack_gpu = uxTaskGetStackHighWaterMark(xTaskGetHandle("loop_gpu")); // GPU core

	print_profiler_stats();
	end_profile();
}
