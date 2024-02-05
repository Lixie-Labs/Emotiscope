//---------------------------------------------------------
//                      _                            _
//                     | |                          | |
//  ___   _   _   ___  | |_    ___   _ __ ___       | |__
// / __| | | | | / __| | __|  / _ \ | '_ ` _ \      | '_ \ 
// \__ \ | |_| | \__ \ | |_  |  __/ | | | | | |  _  | | | |
// |___/  \__, | |___/  \__|  \___| |_| |_| |_| (_) |_| |_|
//         __/ |
//        |___/
//
// Foundational functions like UART initialization

bool silence_detected = true;
float silence_level = 1.0;

void init_serial(uint32_t baud_rate) {
	uint16_t profiler_index = start_function_timing(__func__);

	Serial.begin(baud_rate);
	Serial.println('\n');
	Serial.println("######################");
	Serial.println("SENSORY OVERLOAD REDUX");
	Serial.println("######################");

	end_function_timing(profiler_index);
}

void init_system() {
	uint16_t profiler_index = start_function_timing(__func__);

	extern void init_leds();
	extern void init_i2s_microphone();
	extern void init_window_lookup();
	extern void init_goertzel_constants_musical();
	extern void init_goertzel_constants_full_spectrum();
	extern void init_tempo_goertzel_constants();
	extern void init_wifi();
	extern void init_filesystem();

	// Artificial boot up wait time
	//for(uint16_t i = 0; i < 5; i++){ printf("WAITING FOR %d SECONDS...\n", 5-i); delay(1000); }

	init_serial(2000000);					  // (system.h)
	init_filesystem();                        // (filesystem.h)
	init_configuration();                     // (configuration.h)
	init_profiler();						  // (profiler.h)
	init_leds();							  // (leds.h)
	init_i2s_microphone();					  // (microphone.h)
	init_window_lookup();					  // (goertzel.h)
	init_goertzel_constants_musical();		  // (goertzel.h)
	init_tempo_goertzel_constants();		  // (tempo.h)
	init_wifi();                              // (wireless.h)

	load_sliders_relevant_to_mode(configuration.current_mode);
	load_toggles_relevant_to_mode(configuration.current_mode);

	end_function_timing(profiler_index);
}