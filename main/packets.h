

void broadcast_emotiscope_state(){
	start_profile(__COUNTER__, __func__);
	extern httpd_handle_t server;
	extern bool connected_to_wifi;
	extern uint16_t NUM_LIGHT_MODES;
	extern esp_err_t wstx_broadcast(const char *message);

	if(server != NULL && connected_to_wifi == true){
		static int64_t last_broadcast_time = 0;
		if(t_now_ms - last_broadcast_time >= 250){
			last_broadcast_time = t_now_ms;

			char output_string[2048];
			char temp_buffer[128];

			// Stats
			dsps_memset_aes3(output_string, 0, 2048);
			dsps_memset_aes3(temp_buffer, 0, 128);

			strcat(output_string, "EMO");
			strcat(output_string, "~stats");

			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "|cpu_usage|%.3f", CPU_CORE_USAGE);
			strcat(output_string, temp_buffer);

			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "|cpu_temp|%.3f", CPU_TEMP);
			strcat(output_string, temp_buffer);

			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "|fps_cpu|%.3f", FPS_CPU);
			strcat(output_string, temp_buffer);

			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "|fps_gpu|%.3f", FPS_GPU);
			strcat(output_string, temp_buffer);

			dsps_memset_aes3(temp_buffer, 0, 128);
			sprintf(temp_buffer, "|heap|%zu", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
			strcat(output_string, temp_buffer);

			wstx_broadcast(output_string);
			
			// Configuration
			dsps_memset_aes3(output_string, 0, 2048);
			dsps_memset_aes3(temp_buffer, 0, 128);

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
					dsps_memset_aes3(temp_buffer, 0, 128);
					sprintf(temp_buffer, "%lu", item.value.u32);
					strcat(output_string, temp_buffer);
				}
				else if(item.type == i32t){
					dsps_memset_aes3(temp_buffer, 0, 128);
					sprintf(temp_buffer, "%li", item.value.i32);
					strcat(output_string, temp_buffer);
				}
				else if(item.type == f32t){
					dsps_memset_aes3(temp_buffer, 0, 128);
					sprintf(temp_buffer, "%.3f", item.value.f32);
					strcat(output_string, temp_buffer);
				}
				else{
					//ESP_LOGE(TAG, "ERROR: Unknown type in broadcast_emotiscope_state");
				}
			}

			wstx_broadcast(output_string);

			// Light Modes
			dsps_memset_aes3(output_string, 0, 2048);
			dsps_memset_aes3(temp_buffer, 0, 128);

			strcat(output_string, "EMO");
			strcat(output_string, "~modes");

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

			wstx_broadcast(output_string);
		}
	}

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
	////ESP_LOGI(TAG, "Parsing Emotiscope Packet...");
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
		else{
			if(chunk_type[0] == '\0'){
				////ESP_LOGI(TAG, "PACKET HEADER: %s", section_buffer);
			}
			else{
				////ESP_LOGE(TAG, "UNKNOWN CHUNK TYPE: %s", chunk_type);
			}
		}

		if(chunk_ended == true){
			//printf("Chunk ended.\n");

			if(byte_index < num_bytes){
				// Prepare for new chunk type
				memset(chunk_type, 0, 32);
				load_section_at(section_buffer, websocket_packet_buffer, &byte_index);
				memcpy(chunk_type, section_buffer, 32);
				////ESP_LOGI(TAG, "PACKET CHUNK TYPE: %s", chunk_type);

				config_data_index = 0;
			}
			else{
				////ESP_LOGI(TAG, "PACKET EOF");
			}
		}
	}
}