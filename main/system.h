void init_system(){
	ESP_LOGI(TAG, "Booting up Emotiscope");

	init_configuration();
	init_wifi();
}