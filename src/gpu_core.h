// -------------------------------------------------------------------------
//                                                                    _
//                                                                   | |
//   __ _   _ __    _   _             ___    ___    _ __    ___      | |__
//  / _` | | '_ \  | | | |           / __|  / _ \  | '__|  / _ \     | '_ \ 
// | (_| | | |_) | | |_| |          | (__  | (_) | | |    |  __/  _  | | | |
//  \__, | | .__/   \__,_|           \___|  \___/  |_|     \___| (_) |_| |_|
//   __/ | | |              ______
//  |___/  |_|             |______|
//
// Main loop of the GPU core (Core 0)

float lpf_drag = 0.0;

void run_gpu() {
	uint16_t profiler_index = start_function_timing(__func__);

	uint32_t t_start_cycles = ESP.getCycleCount();

	// Get the current time in microseconds and milliseconds
	static uint32_t t_last_us = micros();
	uint32_t t_now_us = micros();
	uint32_t t_now_ms = t_now_us / 1000.0;

	// Calculate the "delta" value, to scale movements based on FPS, like a game
	// engine
	const uint32_t ideal_us_interval = (1000000 / REFERENCE_FPS);
	int32_t t_elapsed_us = t_now_us - t_last_us;
	float delta = float(t_elapsed_us) / ideal_us_interval;

	// Save the current timestamp for next loop
	t_last_us = t_now_us;

	// Update the novelty curve
	if (magnitudes_locked == false) {
		update_novelty(t_now_us);  // (tempo.h)
	}

	// Update the tempi phases
	update_tempi_phase(delta);	// (tempo.h)

	// RUN THE CURRENT MODE
	// ------------------------------------------------------------
	if (magnitudes_locked == false) {
		clear_display();
		lightshow_modes[configuration.current_mode].draw();

		// If silence is detected, show a blue debug LED
		// leds[NUM_LEDS - 1] = add(leds[NUM_LEDS - 1], {0.0, 0.0, silence_level});

		// Apply an incandescent LUT to reduce harsh blue tones
		apply_incandescent_filter(configuration.incandescent_filter);  // (leds.h)

		// Restrict CRGBF values to 0.0-1.0 range
		clip_leds();  // (leds.h)

		// Save this frame for the next loop
		// save_leds_to_last(); // (leds.h)

		// Smooth the output via EMA
		// volatile float frame_blending_strength = 0.75;
		// smooth_led_output(frame_blending_strength);

		apply_brightness();

		// Render the current debug value as a dot
		render_debug_value(t_now_ms);  // (leds.h)

		float lpf_cutoff_frequency = configuration.speed;
		lpf_cutoff_frequency = lpf_cutoff_frequency * (1.0 - lpf_drag) + 0.5 * lpf_drag;
		lpf_drag *= 0.995;

		apply_image_lpf(lpf_cutoff_frequency);
	}

	// Quantize the floating point color to 8-bit with dithering
	quantize_color();  // (leds.h)

	// Output the quantized color to the 8-bit LED strand
	// uint16_t fastled_profiler_index = start_function_timing("FastLED.show()");
	// FastLED.show();
	transmit_leds();
	// end_function_timing(fastled_profiler_index);

	// Update the FPS_GPU variable
	watch_gpu_fps(t_now_us);  // (system.h)

	uint32_t t_end_cycles = ESP.getCycleCount();
	volatile uint32_t gpu_total_cycles = t_end_cycles - t_start_cycles;

	end_function_timing(profiler_index);
}