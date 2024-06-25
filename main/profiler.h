int64_t t_now_ms = 0;
int64_t t_now_us = 0;

float FPS_CPU_SAMPLES[16];
float FPS_GPU_SAMPLES[16];

float CPU_CORE_USAGE = 0.0;
float FPS_CPU = 0.0;
float FPS_GPU = 0.0;
float CPU_TEMP = 0.0;
uint32_t FREE_HEAP = 0;

void watch_cpu_fps() {
	int64_t us_now = esp_timer_get_time();
	static int64_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	int64_t elapsed_us = us_now - last_call;
	FPS_CPU_SAMPLES[average_index % 16] = 1000000.0 / (float)elapsed_us;
	last_call = us_now;
}

void watch_gpu_fps() {
	int64_t us_now = esp_timer_get_time();
	static int64_t last_call;
	static uint8_t average_index = 0;
	average_index++;

	int64_t elapsed_us = us_now - last_call;
	FPS_GPU_SAMPLES[average_index % 16] = 1000000.0 / (float)elapsed_us;

	last_call = us_now;
}

void update_stats() {
	const uint16_t update_hz = 10;
	const uint32_t update_interval = 1000 / update_hz;
	static int64_t last_update = 0;

	if (t_now_ms - last_update < update_interval) {
		return;
	}

	FPS_CPU = 0.0;
	FPS_GPU = 0.0;
	for (uint8_t i = 0; i < 16; i++) {
		FPS_CPU += FPS_CPU_SAMPLES[i];
		FPS_GPU += FPS_GPU_SAMPLES[i];
	}
	FPS_CPU /= 16.0;
	FPS_GPU /= 16.0;

	CPU_TEMP = get_cpu_temperature();

	FREE_HEAP = heap_caps_get_largest_free_block(MALLOC_CAP_32BIT);

	//UBaseType_t free_stack_cpu = uxTaskGetStackHighWaterMark(NULL); // CPU core (this one)
	//UBaseType_t free_stack_gpu = uxTaskGetStackHighWaterMark(xTaskGetHandle("loop_gpu")); // GPU core
}