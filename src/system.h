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

volatile bool EMOTISCOPE_ACTIVE = true;

void init_serial(uint32_t baud_rate) {
	// Artificial 10-second boot up wait time if needed
	//for(uint16_t i = 0; i < 10; i++){ printf("WAITING FOR %d SECONDS...\n", 10-i); delay(1000); }

	// Get the ESP-IDF version
    const char* idf_version = esp_get_idf_version();

	Serial.begin(baud_rate);
	Serial.println('\n');
	Serial.println("######################");
	Serial.printf( "EMOTISCOPE v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    Serial.printf( "ESP-IDF Version: %s\n", IDF_VER);
	Serial.println("######################");
}

void init_system() {
	extern void init_leds();
	extern void init_i2s_microphone();
	extern void init_window_lookup();
	extern void init_goertzel_constants_musical();
	extern void init_goertzel_constants_full_spectrum();
	extern void init_tempo_goertzel_constants();
	extern void init_wifi();
	extern void init_filesystem();
	extern void init_rmt_driver();
	extern void init_indicator_light();

	init_serial(2000000);					  // (system.h)
	init_filesystem();                        // (filesystem.h)
	init_configuration();                     // (configuration.h)
	init_i2s_microphone();					  // (microphone.h)
	init_window_lookup();					  // (goertzel.h)
	init_goertzel_constants_musical();		  // (goertzel.h)
	init_tempo_goertzel_constants();		  // (tempo.h)	
	init_indicator_light();                   // (indicator.h)
	init_rmt_driver();                        // (led_driver.h)
	init_wifi();                              // (wireless.h)

	// Load sliders 
	load_sliders_relevant_to_mode(configuration.current_mode);

	// Load toggles
	load_toggles_relevant_to_mode(configuration.current_mode);
}