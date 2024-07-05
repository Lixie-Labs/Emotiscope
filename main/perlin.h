float perlin_noise_array[NUM_LEDS>>2];

unsigned int hash(unsigned int x, unsigned int seed) {
    const unsigned int m = 0x5bd1e995U;
    unsigned int hash = seed;
    unsigned int k = x;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;
    return hash;
}

unsigned int hash_vec2(uvec2 x, unsigned int seed) {
    const unsigned int m = 0x5bd1e995U;
    unsigned int hash = seed;
    unsigned int k;

    k = x.x;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;

    k = x.y;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;

    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;
    return hash;
}

vec2 gradient_direction(unsigned int hash) {
    switch (hash & 7) {
    case 0:
        return (vec2){1, 1};
    case 1:
        return (vec2){-1, 1};
    case 2:
        return (vec2){1, -1};
    case 3:
        return (vec2){-1, -1};
    case 4:
        return (vec2){1, 0};
    case 5:
        return (vec2){-1, 0};
    case 6:
        return (vec2){0, 1};
    case 7:
        return (vec2){0, -1};
    }

    return (vec2){0, 0};
}

float interpolate_perlin(float value1, float value2, float value3, float value4, vec2 t) {
    float mix1 = value1 + t.x * (value2 - value1);
    float mix2 = value3 + t.x * (value4 - value3);
    return mix1 + t.y * (mix2 - mix1);
}

vec2 fade(vec2 t) {
    return (vec2){t.x * t.x * t.x * (t.x * (t.x * 6.0 - 15.0) + 10.0), t.y * t.y * t.y * (t.y * (t.y * 6.0 - 15.0) + 10.0)};
}

float dot(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float perlin_noise(vec2 position, unsigned int seed) {
    vec2 floor_position = (vec2){floor(position.x), floor(position.y)};
    vec2 fract_position = (vec2){position.x - floor_position.x, position.y - floor_position.y};
    uvec2 cell_coordinates = (uvec2){(unsigned int)floor_position.x, (unsigned int)floor_position.y};
    
    float value1 = dot(gradient_direction(hash_vec2(cell_coordinates, seed)), fract_position);
    float value2 = dot(gradient_direction(hash_vec2((uvec2){cell_coordinates.x + 1, cell_coordinates.y}, seed)), (vec2){fract_position.x - 1, fract_position.y});
    float value3 = dot(gradient_direction(hash_vec2((uvec2){cell_coordinates.x, cell_coordinates.y + 1}, seed)), (vec2){fract_position.x, fract_position.y - 1});
    float value4 = dot(gradient_direction(hash_vec2((uvec2){cell_coordinates.x + 1, cell_coordinates.y + 1}, seed)), (vec2){fract_position.x - 1, fract_position.y - 1});
    
    return interpolate_perlin(value1, value2, value3, value4, fade(fract_position));
}

float perlin_noise_octaves(vec2 position, int frequency, int octave_count, float persistence, float lacunarity, unsigned int seed) {
    float value = 0.0;
    float amplitude = 1.0;
    float current_frequency = (float)frequency;
    unsigned int current_seed = seed;

    for (int i = 0; i < octave_count; i++) {
        current_seed = hash(current_seed, 0x0U); // create a new seed for each octave
        value += perlin_noise((vec2){position.x * current_frequency, position.y * current_frequency}, current_seed) * amplitude;
        amplitude *= persistence;
        current_frequency *= lacunarity;
    }
    return value;
}

void generate_perlin_noise(float position_x, float position_y, uint32_t seed, float frequency, float persistence, float lacunarity, uint16_t octave_count) {
    for (int i = 0; i < NUM_LEDS>>2; i++) {
        vec2 pos = {position_x + num_leds_float_lookup[i<<2], position_y};
        float noise_value = perlin_noise_octaves(pos, frequency, octave_count, persistence, lacunarity, seed);
        perlin_noise_array[i] = noise_value;
    }

	dsps_addc_f32(perlin_noise_array, perlin_noise_array, NUM_LEDS>>2, 1.0, 1, 1);
	dsps_mulc_f32(perlin_noise_array, perlin_noise_array, NUM_LEDS>>2, 0.5, 1, 1);
}

void update_perlin_noise(float delta){
	start_profile(__COUNTER__, __func__);

	if(configuration.color_mode.value.u32 != COLOR_MODE_PERLIN){
		end_profile();
		return;
	}

	const int64_t update_ms_interval = 10;
	static int64_t last_update_ms = 0;
	if (t_now_ms - last_update_ms < update_ms_interval) {
		end_profile();
		return;
	}
	
	const static float frequency = 2; // Base frequency
    const static float persistence = 0.5; // Amplitude factor for each octave
    const static float lacunarity = 2.0; // Frequency factor for each octave
    const static int octave_count = 2; // Reduce the number of octaves for smoother noise
	const static unsigned int seed = 0x578437adU;

	static float position_x = 0.0;
	static float position_y = 0.0;

	position_y += (0.001 * delta);

    generate_perlin_noise(position_x, position_y, seed, frequency, persistence, lacunarity, octave_count);

	end_profile();
}