void run_web() {
	profile_function([&]() {
		uint32_t t_now_ms = millis();

		handle_wifi();
		if (web_server_ready == true) {
			process_command_queue(t_now_ms);
			discovery_check_in();

			// Write pending changes to LittleFS
			sync_configuration_to_file_system(t_now_ms);
		}
	}, __func__ );
}