void init_system(){
	ESP_LOGI(TAG, "init_system()");

	// Load configuration from NVS
	init_configuration();

	// Attempt connection to WiFi network
	init_wifi();
}