// Initialize the entire system before starting the main loops
void init_system(){
	ESP_LOGI(TAG, "init_system()");

	// Load configuration from NVS (configuration.h)
	init_configuration();

	// Attempt connection to WiFi network (wireless.h)
	init_wifi();
}