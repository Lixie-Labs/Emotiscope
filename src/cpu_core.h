// ------------------------------------------------------------------------
//                                                                   _
//                                                                  | |
//   ___   _ __    _   _             ___    ___    _ __    ___      | |__
//  / __| | '_ \  | | | |           / __|  / _ \  | '__|  / _ \     | '_ \ 
// | (__  | |_) | | |_| |          | (__  | (_) | | |    |  __/  _  | | | |
//  \___| | .__/   \__,_|           \___|  \___/  |_|     \___| (_) |_| |_|
//        | |              ______
//        |_|             |______|
//
// Main loop of the CPU core (Core 1)

void run_cpu() {
	//------------------------------------------------------------------------------------------
	// TIMING
	// ----------------------------------------------------------------------------------
	uint16_t profiler_index = start_function_timing(__func__);
	static uint32_t iter = 0;
	iter++;

	// Get the current time in microseconds and milliseconds
	uint32_t t_now_us = micros();
	uint32_t t_now_ms = t_now_us / 1000;

	//------------------------------------------------------------------------------------------
	// AUDIO CALCULATIONS
	// ----------------------------------------------------------------------

	// Get new audio chunk from the I2S microphone
	acquire_sample_chunk();	 // (microphone.h)

	uint32_t processing_start_us = micros();

	// Calculate the magnitude of the currently studied frequency set
	calculate_magnitudes();  // (goertzel.h)

//printf("update_tempo() = %.4fus\n", measure_execution([&]() {
	// Log novelty and calculate the most probable tempi
	update_tempo();	 // (tempo.h)
//}));

	// Update the FPS_CPU variable
	watch_cpu_fps(t_now_us);  // (system.h)

	// Occasionally print the average frame rate
	print_fps_values(t_now_ms);

	// Occasionally print connected WS clients
	print_websocket_clients(t_now_ms);

	// Write pending changes to LittleFS
	sync_configuration_to_file_system(t_now_ms);

	// print_audio_data();

	//------------------------------------------------------------------------------------------
	// WIFI
	// ------------------------------------------------------------------------------------
	run_wireless();	 // (wireless.h)

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
	if (iter % 500 == 0) {
		printf("MAIN CPU CORE USAGE: %.4f\n", audio_frame_to_processing_ratio);
		print_memory_info();
	}

	//------------------------------------------------------------------------------------------
	yield();  // Keep CPU watchdog happy
	end_function_timing(profiler_index);
}