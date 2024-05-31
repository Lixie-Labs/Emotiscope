void run_web() {
	profile_function([&]() {
		handle_wifi();
		dns_server.processNextRequest();

		if (web_server_ready == true && wifi_config_mode == false) {
			discovery_check_in();

			// Write pending changes to LittleFS
			sync_configuration_to_file_system();

			if(configuration_changed == true) {
				configuration_changed = false;
			}

			static uint32_t last_websocket_client_test_ms = 0;
			if (t_now_ms - last_websocket_client_test_ms >= 100) {
				last_websocket_client_test_ms = t_now_ms;
				test_clients();
			}

			// Broadcast the current state to all clients
			static uint32_t last_state_broadcast_ms = 0;
			if (t_now_ms - last_state_broadcast_ms >= 250) {
				last_state_broadcast_ms = t_now_ms;

				//uint32_t t_start = micros();
				broadcast_emotiscope_state();
				//uint32_t t_end = micros();

				//printf("STATE TIME: %luus\n", t_end - t_start);
				//printf("STATE HZ: %f\n", 1000000.0 / (float)(t_end - t_start));
			}
		}
	}, __func__ );
}