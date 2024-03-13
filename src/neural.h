#define INPUT_LAYER_SIZE    64
#define HIDDEN_LAYER_1_SIZE 64
#define HIDDEN_LAYER_2_SIZE 64
#define HIDDEN_LAYER_3_SIZE 64
#define OUTPUT_LAYER_SIZE   64

float input_neuron_values    [INPUT_LAYER_SIZE];
float input_neuron_weights   [INPUT_LAYER_SIZE];
float input_neuron_biases    [INPUT_LAYER_SIZE];

float hidden_neuron_1_values [HIDDEN_LAYER_1_SIZE];
float hidden_neuron_1_weights[HIDDEN_LAYER_1_SIZE];
float hidden_neuron_1_biases [HIDDEN_LAYER_1_SIZE];

float hidden_neuron_2_values [HIDDEN_LAYER_2_SIZE];
float hidden_neuron_2_weights[HIDDEN_LAYER_2_SIZE];
float hidden_neuron_2_biases [HIDDEN_LAYER_2_SIZE];

float hidden_neuron_3_values [HIDDEN_LAYER_3_SIZE];
float hidden_neuron_3_weights[HIDDEN_LAYER_3_SIZE];
float hidden_neuron_3_biases [HIDDEN_LAYER_3_SIZE];

float output_neuron_values[OUTPUT_LAYER_SIZE];

void init_random_weights(){
	randomSeed(analogRead(1));
	// Initialize all input weights and biases to random values between -1 and 1
	for(int i = 0; i < INPUT_LAYER_SIZE; i++){
		input_neuron_weights[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
		input_neuron_biases[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
	}
	for(int i = 0; i < HIDDEN_LAYER_1_SIZE; i++){
		hidden_neuron_1_weights[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
		hidden_neuron_1_biases[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
	}
	for(int i = 0; i < HIDDEN_LAYER_2_SIZE; i++){
		hidden_neuron_2_weights[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
		hidden_neuron_2_biases[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
	}
	for(int i = 0; i < HIDDEN_LAYER_3_SIZE; i++){
		hidden_neuron_3_weights[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
		hidden_neuron_3_biases[i] = (float)rand() / (float)RAND_MAX * 2 - 1;
	}
}

void neural_network_feed_forward(){
	uint32_t t_start = micros();

	memcpy(input_neuron_values, spectrogram_smooth, sizeof(float) * INPUT_LAYER_SIZE);

	// input to hidden 1
	for(int i = 0; i < HIDDEN_LAYER_1_SIZE; i++){
		dsps_dotprod_f32_ae32(input_neuron_values, input_neuron_weights, &hidden_neuron_1_values[i], INPUT_LAYER_SIZE);
		if(hidden_neuron_1_values[i] < 0){
			hidden_neuron_1_values[i] = 0;
		}
		hidden_neuron_1_values[i] += input_neuron_biases[i];
	}

	// hidden 1 to hidden 2
	for(int i = 0; i < HIDDEN_LAYER_2_SIZE; i++){
		dsps_dotprod_f32_ae32(hidden_neuron_1_values, hidden_neuron_1_weights, &hidden_neuron_2_values[i], HIDDEN_LAYER_1_SIZE);
		if(hidden_neuron_2_values[i] < 0){
			hidden_neuron_2_values[i] = 0;
		}
		hidden_neuron_2_values[i] += hidden_neuron_2_biases[i];
	}

	// hidden 2 to hidden 3
	for(int i = 0; i < HIDDEN_LAYER_3_SIZE; i++){
		dsps_dotprod_f32_ae32(hidden_neuron_2_values, hidden_neuron_2_weights, &hidden_neuron_3_values[i], HIDDEN_LAYER_2_SIZE);
		if(hidden_neuron_3_values[i] < 0){
			hidden_neuron_3_values[i] = 0;
		}
		hidden_neuron_3_values[i] += hidden_neuron_3_biases[i];
	}

	// hidden 3 to output with range 0 to 1 using sigmoid
	for(int i = 0; i < OUTPUT_LAYER_SIZE; i++){
		dsps_dotprod_f32_ae32(hidden_neuron_3_values, hidden_neuron_3_weights, &output_neuron_values[i], HIDDEN_LAYER_3_SIZE);
		output_neuron_values[i] = 1 / (1 + exp(-output_neuron_values[i] - hidden_neuron_3_biases[i]));
	}

	uint32_t t_end = micros();
	uint32_t t_duration = t_end - t_start;
	float FPS = 1000000.0 / t_duration;

	/*
	printf("Output values: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n", 
		output_neuron_values[0], output_neuron_values[1], output_neuron_values[2], output_neuron_values[3], 
		output_neuron_values[4], output_neuron_values[5], output_neuron_values[6], output_neuron_values[7], 
		output_neuron_values[8], output_neuron_values[9], output_neuron_values[10], output_neuron_values[11]);
	*/
	printf("Neural network feed forward took %d us, FPS: %f\n", t_duration, FPS);
}