void draw_perlin(){
	unsigned int seed = 0x578437adU;

	static float position_x = 0.0;
	static float position_y = 0.0;

	position_y += 0.01;

	float noise_array[NUM_LEDS];
    float frequency = 1.0; // Base frequency
    float persistence = 0.5; // Amplitude factor for each octave
    float lacunarity = 2.0; // Frequency factor for each octave
    int octave_count = 3; // Reduce the number of octaves for smoother noise

    generate_perlin_noise(noise_array, NUM_LEDS, position_x, position_y, seed, frequency, persistence, lacunarity, octave_count);

	for (int i = 0; i < NUM_LEDS; i++) {
		float progress = num_leds_float_lookup[i];
		CRGBF color = hsv(
			noise_array[i]*noise_array[i],
			configuration.saturation.value.f32,
			0.25
		);
		
		leds[i] = color;
	}
}