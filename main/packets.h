char emotiscope_packet_buffer[1024];
uint16_t emotiscope_packet_buffer_index = 0;

void append_to_packet(char* data){
	//ESP_LOGI(TAG, "Appending to packet: %s", data);

	int16_t data_length = strlen(data);
	memcpy(emotiscope_packet_buffer + emotiscope_packet_buffer_index, data, data_length);
	emotiscope_packet_buffer_index += data_length;
}

void broadcast_emotiscope_state(httpd_req_t* req){
	start_profile(__COUNTER__, __func__);

	//ESP_LOGI(TAG, "broadcast_emotiscope_state");

	extern httpd_handle_t server;
	extern bool connected_to_wifi;
	extern uint16_t NUM_LIGHT_MODES;
	char temp_buffer[128];

	// Stats
	dsps_memset_aes3(emotiscope_packet_buffer, 0, 1024);
	emotiscope_packet_buffer_index = 0;
	dsps_memset_aes3(temp_buffer, 0, 128);
	append_to_packet("EMO~stats");

	dsps_memset_aes3(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|cpu_usage|%.3f", CPU_CORE_USAGE);
	append_to_packet(temp_buffer);

	dsps_memset_aes3(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|cpu_temp|%.3f", CPU_TEMP);
	append_to_packet(temp_buffer);

	dsps_memset_aes3(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|fps_cpu|%.3f", FPS_CPU);
	append_to_packet(temp_buffer);

	dsps_memset_aes3(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|fps_gpu|%.3f", FPS_GPU);
	append_to_packet(temp_buffer);

	dsps_memset_aes3(temp_buffer, 0, 128);
	sprintf(temp_buffer, "|heap|%zu", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
	append_to_packet(temp_buffer);

	wstx( req, emotiscope_packet_buffer );
	
	// Configuration
	dsps_memset_aes3(emotiscope_packet_buffer, 0, 1024);
	emotiscope_packet_buffer_index = 0;
	dsps_memset_aes3(temp_buffer, 0, 128);
	append_to_packet("EMO~config");

	config_item* config_location = (config_item*)(&configuration);
	uint16_t num_config_items = sizeof(configuration) / sizeof(config_item);

	for(uint16_t i = 0; i < num_config_items; i++){
		config_item item = *(config_location + i);

		append_to_packet("|");
		append_to_packet(item.pretty_name);
		append_to_packet("|");
		append_to_packet(item.type_string);
		append_to_packet("|");
		append_to_packet(item.ui_type_string);
		append_to_packet("|");

		if(item.type == u32t){
			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%lu", item.value.u32);
			append_to_packet(temp_buffer);
		}
		else if(item.type == i32t){
			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%li", item.value.i32);
			append_to_packet(temp_buffer);
		}
		else if(item.type == f32t){
			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "%.3f", item.value.f32);
			append_to_packet(temp_buffer);
		}
		else{
			ESP_LOGE(TAG, "ERROR: Unknown type in broadcast_emotiscope_state");
		}
	}

	wstx( req, emotiscope_packet_buffer );

	// Light Modes
	dsps_memset_aes3(emotiscope_packet_buffer, 0, 1024);
	emotiscope_packet_buffer_index = 0;
	dsps_memset_aes3(temp_buffer, 0, 128);
	append_to_packet("EMO~modes");

	for(uint16_t i = 0; i < NUM_LIGHT_MODES; i++){
		light_mode_type_t type = light_modes[i].type;
		if(type != LIGHT_MODE_TYPE_SYSTEM){
			append_to_packet("|");
			append_to_packet(light_modes[i].name);
			append_to_packet("|");

			if(type == LIGHT_MODE_TYPE_ACTIVE){
				append_to_packet("0");
			}
			else if(type == LIGHT_MODE_TYPE_INACTIVE){
				append_to_packet("1");
			}
			else{
				ESP_LOGE(TAG, "ERROR: Unknown light mode type in broadcast_emotiscope_state\n");
			}
		}
	}

	wstx( req, emotiscope_packet_buffer );

	end_profile(__COUNTER__, __func__);
}

void broadcast_debug_graph(httpd_req_t* req){
	start_profile(__COUNTER__, __func__);

	extern float graph_data[MAX_GRAPH_SIZE];
	extern uint8_t graph_data_length;

	char temp_buffer[512];

	// Debug Graph
	dsps_memset_aes3(emotiscope_packet_buffer, 0, 1024);
	emotiscope_packet_buffer_index = 0;
	dsps_memset_aes3(temp_buffer, 0, 512);
	append_to_packet("EMO~graph");

	dsps_memset_aes3(temp_buffer, 0, 512);
	sprintf(temp_buffer, "|%d|", graph_data_length);
	append_to_packet(temp_buffer);

	for(uint16_t i = 0; i < graph_data_length; i+=4){
		dsps_memset_aes3(temp_buffer, 0, 512);
		sprintf(
			temp_buffer,
			"%d,%d,%d,%d,",
			(uint8_t)lroundf(graph_data[i+0] * 255.0f),
			(uint8_t)lroundf(graph_data[i+1] * 255.0f),
			(uint8_t)lroundf(graph_data[i+2] * 255.0f),
			(uint8_t)lroundf(graph_data[i+3] * 255.0f)
		);
		append_to_packet(temp_buffer);
	}

	wstx( req, emotiscope_packet_buffer );

	end_profile(__COUNTER__, __func__);
}

bool load_section_at( char* section_buffer, char* command, int16_t* byte_index ){
	memset(section_buffer, 0, 128);

	int16_t output_index = 0;
	bool solved = false;
	bool hit_end_of_chunk = false;

	while(solved == false){
		char current_byte = command[*byte_index];

		if(current_byte == '~' || current_byte == '\0'){ // 0 is EOF
			hit_end_of_chunk = true;
			solved = true;
		}
		else if(current_byte == '|'){
			hit_end_of_chunk = false;
			solved = true;
		}
		else{
			// read data
			section_buffer[output_index] = current_byte;
			output_index++;
		}

		(*byte_index)++;
	}

	//printf("LOADED SECTION: %s\n", section_buffer);

	return hit_end_of_chunk;
}

void parse_emotiscope_packet(httpd_req_t* req){
	//ESP_LOGI(TAG, "Parsing Emotiscope Packet...");
	int16_t num_bytes = strlen(websocket_packet_buffer);
	int16_t byte_index = 0;

	char section_buffer[128];
	memset(section_buffer, 0, 128);

	char chunk_type[32];
	memset(chunk_type, 0, 32);

	int16_t config_data_index = 0;

	while(byte_index <= num_bytes){
		bool chunk_ended = load_section_at(section_buffer, websocket_packet_buffer, &byte_index);

		if(fastcmp(chunk_type, "set_config")){
			//ESP_LOGI(TAG, "SET CONFIG: %s", section_buffer);
			static char config_pretty_name[32];
			static char new_config_value[32];

			if(config_data_index == 0){
				memcpy(config_pretty_name, section_buffer, 32);
				config_data_index = 1;
			}
			else if(config_data_index == 1){
				memcpy(new_config_value, section_buffer, 32);
				config_data_index = 0;

				set_config_value_by_pretty_name(config_pretty_name, new_config_value);
				save_config_delayed();
			}
		}
		else if(fastcmp(chunk_type, "get_state")){
			//ESP_LOGI(TAG, "GET STATE: %s", section_buffer);
			broadcast_emotiscope_state(req);
		}
		else if(fastcmp(chunk_type, "get_graph")){
			//ESP_LOGI(TAG, "GET STATE: %s", section_buffer);
			broadcast_debug_graph(req);
		}
		else{
			if(chunk_type[0] == '\0'){
				//ESP_LOGI(TAG, "PACKET HEADER: %s", section_buffer);
			}
			else{
				//ESP_LOGE(TAG, "UNKNOWN CHUNK TYPE: %s", chunk_type);
			}
		}

		if(chunk_ended == true){
			//ESP_LOGI(TAG, "Chunk ended");

			if(byte_index < num_bytes){
				// Prepare for new chunk type
				memset(chunk_type, 0, 32);
				load_section_at(section_buffer, websocket_packet_buffer, &byte_index);
				memcpy(chunk_type, section_buffer, 32);
				//ESP_LOGI(TAG, "PACKET CHUNK TYPE: %s", chunk_type);

				config_data_index = 0;
			}
			else{
				//ESP_LOGI(TAG, "PACKET EOF");
			}
		}
	}
}