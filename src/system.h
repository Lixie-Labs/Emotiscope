/*
--------------------------------------------------------
                     _                            _
                    | |                          | |
 ___   _   _   ___  | |_    ___   _ __ ___       | |__
/ __| | | | | / __| | __|  / _ \ | '_ ` _ \      | '_ \ 
\__ \ | |_| | \__ \ | |_  |  __/ | | | | | |  _  | | | |
|___/  \__, | |___/  \__|  \___| |_| |_| |_| (_) |_| |_|
        __/ |
       |___/

Foundational functions like UART initialization
*/

volatile bool EMOTISCOPE_ACTIVE = true;

char serial_buffer[256];
uint16_t serial_buffer_index = 0;

uint32_t last_command_time = 0;

void init_serial(uint32_t baud_rate) {
	// Artificial 10-second boot up wait time if needed
	//for(uint16_t i = 0; i < 10; i++){ printf("WAITING FOR %d SECONDS...\n", 10-i); delay(1000); }

	memset(serial_buffer, 0, 256);

	Serial.begin(baud_rate);
}

void check_serial() {
	extern bool queue_command(const char* command, uint16_t length, uint8_t client_slot);
	
	while(Serial.available() > 0){
		char c = Serial.read();
		if(c == '\n'){
			printf("Serial command received: %s\n", serial_buffer);
			queue_command(serial_buffer, serial_buffer_index, 255);
			memset(serial_buffer, 0, 256);
			serial_buffer_index = 0;
		}
		else if(c == '\r'){
			// Do nothing, \r is stupid
			printf("DUMB \\r RECEIVED\n");
		}
		else{
			serial_buffer[serial_buffer_index] = c;
			serial_buffer_index++;
		}
	}
}

void init_boot_button() {
	pinMode(0, INPUT_PULLUP);
}

void check_boot_button(){
	extern void trigger_self_test();
	extern self_test_steps_t self_test_step;

	if(t_now_ms >= 1000){ // Wait 1 second before checking boot button
		if(digitalRead(0) == LOW){ // Boot button is pressed
			if(self_test_step == SELF_TEST_INACTIVE){ // Self test is not already running
				printf("BOOT BUTTON PRESSED, BEGINNING SELF TEST\n");
				EMOTISCOPE_ACTIVE = true; // Wake if needed
				trigger_self_test(); // Begin self test
			}
		}
	}
}

void init_system() {
	extern void init_hardware_version_pins(); // (hardware_version.h)
	extern void init_light_mode_list();       // (light_modes.h)
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
	extern void init_touch();
	extern void init_noise_samples();
	extern void init_floating_point_lookups();

	init_hardware_version_pins();       // (hardware_version.h)
	init_serial(921600);				// (system.h)
	init_light_mode_list();             // (light_modes.h)
	init_filesystem();                  // (filesystem.h)
	init_configuration();               // (configuration.h)
	init_i2s_microphone();				// (microphone.h)
	init_window_lookup();				// (goertzel.h)
	init_goertzel_constants_musical();	// (goertzel.h)
	init_tempo_goertzel_constants();	// (tempo.h)	
	init_indicator_light();             // (indicator.h)
	init_rmt_driver();                  // (led_driver.h)
	init_touch();                       // (touch.h)
	init_wifi();                        // (wireless.h)
	init_noise_samples();               // (utilities.h)
	init_floating_point_lookups();      // (utilities.h)
	init_boot_button();                 // (system.h)

	// Load sliders 
	load_sliders_relevant_to_mode(configuration.current_mode);

	// Load toggles
	load_toggles_relevant_to_mode(configuration.current_mode);
}