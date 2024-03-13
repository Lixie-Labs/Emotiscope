void draw_neurons() {
	for (uint16_t i = 0; i < 64; i++) {
		float hidden_neuron_1_value = clip_float(hidden_neuron_1_values[i]);
		float hidden_neuron_2_value = clip_float(hidden_neuron_2_values[i]);
		float hidden_neuron_3_value = clip_float(hidden_neuron_3_values[i]);
		float output_neuron_value   = clip_float(output_neuron_values[i]);

		CRGBF color = {
			hidden_neuron_2_value * hidden_neuron_2_value,
			hidden_neuron_3_value * hidden_neuron_3_value,
			output_neuron_value * output_neuron_value
		};

		leds[64+i] = color;
		leds[63-i] = color;
	}
}