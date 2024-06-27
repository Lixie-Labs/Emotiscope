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
	// Update the FPS_CPU variable
	watch_cpu_fps();  // (system.h)

	run_indicator_light();

	// Get new audio chunk from the I2S microphone
	acquire_sample_chunk();	 // (microphone.h)

	int64_t processing_start_us = esp_timer_get_time(); // -------------------------------

	// Perform FFT on the new audio data
	perform_fft();	 // (fft.h)	

	// Calculate the magnitudes of the currently studied frequency set
	int64_t t_start = esp_timer_get_time();
	calculate_magnitudes();  // (goertzel.h)
	int64_t t_end = esp_timer_get_time();
	
	static int iter = 0;
	if(iter == 100){
		ESP_LOGI(TAG, "calculate_magnitudes(): %lld us", t_end - t_start);
		iter = 0;
	}
	else{
		iter++;	
	}

	get_chromagram();        // (goertzel.h)

	run_vu(); // (vu.h)

	read_touch(); // (touch.h)

	update_tempo();	 // (tempo.h)

	sync_configuration_to_file_system(); // (configuration.h)

	uint32_t processing_end_us = esp_timer_get_time(); // --------------------------------

	//------------------------------------------------------------------------------------
	// CPU USAGE CALCULATION
	uint32_t processing_us_spent = processing_end_us - processing_start_us;
	uint32_t audio_core_us_per_loop = 1000000.0 / FPS_CPU;
	float audio_frame_to_processing_ratio = processing_us_spent / (float)audio_core_us_per_loop;
	CPU_CORE_USAGE = audio_frame_to_processing_ratio;

	update_stats(); // (profiler.h)


	/*
	run_vu(); // (vu.h)
	update_tempo();	 // (tempo.h)
	read_touch();
	check_boot_button();
	*/
}

void loop_cpu(void *pvParameters) {
	while (1) {
		run_cpu();
		run_cpu();
		run_cpu();
		run_cpu();
	}
}