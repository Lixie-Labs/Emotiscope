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

	// Initialize CPU temperature sensor
	init_cpu_temperature();

	// Load configuration from NVS (configuration.h)
	extern void init_configuration();
	init_configuration();

	// Attempt connection to WiFi network (wireless.h)
	extern void init_wifi();
	init_wifi();

	extern void init_microphone();
	init_microphone();

	extern void init_fft();
	init_fft();
}