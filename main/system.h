volatile bool EMOTISCOPE_ACTIVE = true;

temperature_sensor_handle_t temp_handle = NULL;

// Prepare CPU temperature sensor
void init_cpu_temperature() {
	temperature_sensor_config_t temp_sensor = {
	    .range_min = 20,
	    .range_max = 100,
	};
	ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));

	// Enable temperature sensor
	ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
}

// Get temperature of CPU core
float get_cpu_temperature() {
	float tsens_out;
	ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
	return tsens_out;
}

// Initialize the entire system before starting the main loops
void init_system(){
	ESP_LOGI(TAG, "init_system()");

	// Initialize CPU temperature sensor (system.h)
	init_cpu_temperature();

	// Initialize list of light modes (light_modes.h)
	extern void init_light_mode_list();
	init_light_mode_list();

	// Load configuration from NVS (configuration.h)
	extern void init_configuration();
	init_configuration();

	// Attempt connection to WiFi network (wireless.h)
	extern void init_wifi();
	init_wifi();

	// Initialize the I2S microphone (microphone.h)
	extern void init_microphone(); 
	init_microphone();

	// Initialize the FFT tables (fft.h)
	extern void init_fft();
	init_fft();

	// Initialize the RMT LED driver (led_driver.h)
	extern void init_rmt_driver();
	init_rmt_driver(); 

	// Initialize the indicator light (indicator_light.h)
	extern void init_indicator_light();
	init_indicator_light();

	// Initialize the Goerztel window lookup table (goertzel.h)
	extern void init_window_lookup();
	init_window_lookup();

	// Initialize the Goertzel constants (goertzel.h)
	extern void init_goertzel_constants_musical();
	init_goertzel_constants_musical();

	// Initialize random RNG data (utilities.h)
	extern void init_noise_samples();
	init_noise_samples();

	// Initialize the floating point lookup tables (utilities.h)
	extern void init_floating_point_lookups();
	init_floating_point_lookups();

	// Initialize the touch pins (touch.h)
	extern void init_touch();
	init_touch();

	// Initialize tempo goertzel constants (tempo.h)
	extern void init_tempo_goertzel_constants();
	init_tempo_goertzel_constants();
}