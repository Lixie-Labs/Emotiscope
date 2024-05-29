void run_web() {
	profile_function([&]() {
		handle_wifi();
		dns_server.processNextRequest();

		if (web_server_ready == true && wifi_config_mode == false) {
			discovery_check_in();

			// Write pending changes to LittleFS
			sync_configuration_to_file_system();
		}
	}, __func__ );
}