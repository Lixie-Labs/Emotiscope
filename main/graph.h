float graph_data[MAX_GRAPH_SIZE];
uint8_t graph_data_length = MAX_GRAPH_SIZE;

void run_debug_graph() {
	for(uint8_t i = 0; i < 128; i++){
		float mag = fft_smooth[0][i];
		graph_data[i] = mag;
	}
}