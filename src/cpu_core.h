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

Main loop of the CPU core (Core 1)
*/

void run_cpu() {
	profile_function([&]() {
		//------------------------------------------------------------------------------------------
		// TIMING
		// ----------------------------------------------------------------------------------
		static uint32_t iter = 0;
		iter++;

		//------------------------------------------------------------------------------------------
		// AUDIO CALCULATIONS
		// ----------------------------------------------------------------------

		// Get new audio chunk from the I2S microphone
		acquire_sample_chunk();	 // (microphone.h)

		uint32_t processing_start_us = micros();

		// Calculate the magnitudes of the currently studied frequency set
		calculate_magnitudes();  // (goertzel.h)
		get_chromagram();        // (goertzel.h)

		run_vu(); // (vu.h)

		//printf("update_tempo() = %.4fus\n", measure_execution([&]() {
		// Log novelty and calculate the most probable tempi
		update_tempo();	 // (tempo.h)
		//}));

		// Update the FPS_CPU variable
		watch_cpu_fps();  // (system.h)

		// Occasionally print the average frame rate
		print_system_info();

		// print_audio_data();

		read_touch();

		check_serial();

		check_boot_button();

		//neural_network_feed_forward();

		//------------------------------------------------------------------------------------------
		// WIFI
		// ------------------------------------------------------------------------------------
		//run_wireless();	 // (wireless.h)

		//------------------------------------------------------------------------------------------
		// TESTING AREA, SHOULD BE BLANK IN PRODUCTION

		/*
		printf("MATCH STRCMP    | microseconds taken = %.4f\n", measure_execution([&]() {
			(strcmp("test_string_value", "test_string_value") == 0);
		}));
		*/

		//------------------------------------------------------------------------------------------	

		//------------------------------------------------------------------------------------------
		// CPU USAGE CALCULATION
		// -------------------------------------------------------------------
		uint32_t processing_end_us = micros();
		uint32_t processing_us_spent = processing_end_us - processing_start_us;
		uint32_t audio_core_us_per_loop = 1000000.0 / FPS_CPU;
		float audio_frame_to_processing_ratio = processing_us_spent / float(audio_core_us_per_loop);
		CPU_CORE_USAGE = audio_frame_to_processing_ratio;

		//------------------------------------------------------------------------------------------
		yield();  // Keep CPU watchdog happy

	}, __func__);
}