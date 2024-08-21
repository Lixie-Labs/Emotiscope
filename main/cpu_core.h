/*
------------------------------------------------------------------------
                                                                  _
                                                                 | |
  ___   _ __    _   _             ___    ___    _ __    ___      | |__
 / __| | '_ \  | | | |           / __|  / _ \  | '__|  / _ \     | '_ \ 
| (__  | |_) | | |_| |          | (__  | (_) | | |    |  __/  _  | | | |
 \___| | .__/   \__,_|           \___|  \___/  |_|     \___| (_) |_| |_|
       | |              ______
       |_|             |______|

Main loop of the CPU core
*/

void run_cpu() {
	start_profile(__COUNTER__, __func__);

	// Update the FPS_CPU variable
	watch_cpu_fps();  // (system.h)

	static uint8_t iter = 0;
	iter++;
	if(iter == 10){
		iter = 0;
		run_indicator_light();
		sync_configuration_to_file_system(); // (configuration.h)
	}

	// Get new audio chunk from the I2S microphone
	acquire_sample_chunk();	 // (microphone.h)

	int64_t processing_start_us = esp_timer_get_time(); // -------------------------------

	calculate_magnitudes();  // (goertzel.h)

	perform_fft();	 // (fft.h)	
	
	estimate_pitch(); // (pitch.h)

	get_chromagram();        // (goertzel.h)

	run_vu(); // (vu.h)

	//read_touch(); // (touch.h)

	update_tempo();	 // (tempo.h)

	check_serial();

	uint32_t processing_end_us = esp_timer_get_time(); // --------------------------------

	//------------------------------------------------------------------------------------
	// CPU USAGE CALCULATION
	uint32_t processing_us_spent = processing_end_us - processing_start_us;
	uint32_t audio_core_us_per_loop = 1000000.0 / FPS_CPU;
	float audio_frame_to_processing_ratio = processing_us_spent / (float)audio_core_us_per_loop;
	CPU_CORE_USAGE = audio_frame_to_processing_ratio;

	update_stats(); // (profiler.h)

	//check_boot_button();

	end_profile();
}

void loop_cpu(void *pvParameters) {
	// Initialize all peripherals (system.h) 
	init_system();

	// Start GPU core
	(void)xTaskCreatePinnedToCore(loop_gpu, "loop_gpu", 8192, NULL, 1, NULL, 0);

	while (1) {
		run_cpu();
		run_cpu();
		run_cpu();
		run_cpu();
	}
}