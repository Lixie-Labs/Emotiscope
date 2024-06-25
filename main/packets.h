void broadcast_emotiscope_state(){
	char output_string[2048];
	char temp_buffer[128];

	// Stats
	memset(output_string, 0, 2048);
	memset(temp_buffer, 0, 128);

	strcat(output_string, "EMO");
	strcat(output_string, "~stats");

	memset(temp_buffer, 0, 128);
	//sprintf(temp_buffer, "|cpu_usage|%.3f", CPU_CORE_USAGE);
	sprintf(temp_buffer, "|cpu_usage|0.5");
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	//sprintf(temp_buffer, "|cpu_temp|%.3f", CPU_TEMP);
	sprintf(temp_buffer, "|cpu_temp|69");
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	//sprintf(temp_buffer, "|fps_cpu|%.3f", FPS_CPU);
	sprintf(temp_buffer, "|fps_cpu|200");
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	//sprintf(temp_buffer, "|fps_gpu|%.3f", FPS_GPU);
	sprintf(temp_buffer, "|fps_gpu|400");
	strcat(output_string, temp_buffer);

	memset(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|heap|%zu", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
	strcat(output_string, temp_buffer);

	ESP_LOGI(TAG, "TX: %s", output_string);

	// Configuration
	memset(output_string, 0, 2048);
	memset(temp_buffer, 0, 128);

	strcat(output_string, "EMO");
	strcat(output_string, "~config");

	config_item* config_location = (config_item*)(&configuration);
	uint16_t num_config_items = sizeof(configuration) / sizeof(config_item);

	for(uint16_t i = 0; i < num_config_items; i++){
		config_item item = *(config_location + i);
		
		strcat(output_string, "|");
		strcat(output_string, item.pretty_name);
		strcat(output_string, "|");
		strcat(output_string, item.type_string);
		strcat(output_string, "|");
		strcat(output_string, item.ui_type_string);
		strcat(output_string, "|");

		if(item.type == u32t){
			memset(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%lu", item.value.u32);
			strcat(output_string, temp_buffer);
		}
		else if(item.type == i32t){
			memset(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%li", item.value.i32);
			strcat(output_string, temp_buffer);
		}
		else if(item.type == f32t){
			memset(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%.3f", item.value.f32);
			strcat(output_string, temp_buffer);
		}
		else{
			ESP_LOGE(TAG, "ERROR: Unknown type in broadcast_emotiscope_state");
		}
	}

	ESP_LOGI(TAG, "TX: %s", output_string);

	// Light Modes
	memset(output_string, 0, 2048);
	memset(temp_buffer, 0, 128);

	strcat(output_string, "EMO");
	strcat(output_string, "~modes");

	/*
	for(uint16_t i = 0; i < NUM_LIGHT_MODES; i++){
		light_mode_type_t type = light_modes[i].type;
		if(type != LIGHT_MODE_TYPE_SYSTEM){
			strcat(output_string, "|");
			strcat(output_string, light_modes[i].name);
			strcat(output_string, "|");

			if(type == LIGHT_MODE_TYPE_ACTIVE){
				strcat(output_string, "0");
			}
			else if(type == LIGHT_MODE_TYPE_INACTIVE){
				strcat(output_string, "1");
			}
			else{
				printf("ERROR: Unknown light mode type in broadcast_emotiscope_state\n");
			}
		}
	}
	*/

	ESP_LOGI(TAG, "TX: %s", output_string);
}

// Parse the packet and do something with it
void parse_packet(httpd_req_t* req){
	ESP_LOGI(TAG, "Parsing packet: %s", websocket_packet_buffer);

	wstx(req, websocket_packet_buffer);
}