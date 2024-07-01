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
	static uint32_t iter = 0;
	iter++;

	// Update the FPS_CPU variable
	watch_cpu_fps();  // (system.h)

	run_indicator_light();

	// Get new audio chunk from the I2S microphone
	acquire_sample_chunk();	 // (microphone.h)

	int64_t processing_start_us = esp_timer_get_time(); // -------------------------------

	calculate_magnitudes();  // (goertzel.h)

	if(iter % 2 == 0){
		// Perform FFT on the new audio data
		perform_fft();	 // (fft.h)	
	}
	else{
		estimate_pitch(); // (pitch.h)
	}

	get_chromagram();        // (goertzel.h)

	run_vu(); // (vu.h)

	read_touch(); // (touch.h)

	update_tempo();	 // (tempo.h)

	sync_configuration_to_file_system(); // (configuration.h)

	static int64_t last_test_time = 0;
	if(t_now_ms - last_test_time >= 10000 || last_test_time == 0){
		last_test_time = t_now_ms;
		//run_performance_test(); // (testing.h)
	}

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