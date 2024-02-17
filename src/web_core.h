void run_web() {
	profile_function([&]() {
		handle_wifi();
		if (web_server_ready == true) {
			process_command_queue();
			discovery_check_in();

			// Write pending changes to LittleFS
			sync_configuration_to_file_system();
		}
	}, __func__ );
}