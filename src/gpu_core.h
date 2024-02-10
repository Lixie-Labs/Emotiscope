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
	uint32_t t_start_cycles = ESP.getCycleCount();

	// Get the current time in microseconds and milliseconds
	static uint32_t t_last_us = micros();

	uint32_t t_now_us = micros();
	uint32_t t_now_ms = millis();

	// Calculate the "delta" value, to scale movements based on FPS, like a game engine
	const uint32_t ideal_us_interval = (1000000 / REFERENCE_FPS);
	uint32_t t_elapsed_us = t_now_us - t_last_us;
	float delta = float(t_elapsed_us) / ideal_us_interval;

	// Save the current timestamp for next loop
	t_last_us = t_now_us;

	// Update the novelty curve
	update_novelty(t_now_us);  // (tempo.h)

	// Update the tempi phases
	update_tempi_phase(delta);	// (tempo.h)

	// RUN THE CURRENT MODE
	// ------------------------------------------------------------

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

	// TODO: Screensaver
	// When ever the GPU hits this point in the loop, check if the total luminosity is below a small threshold value,
	// and if so, use 4 reserved draw_dots() in the same color palette as your current mode, and some sine functions
	// to make them "spin around" in a 1-D sense.
	// This can be randomized in fun ways, so that when your album finishes and the room is quiet, the display slowly
	// goes into a slow resting state that still indicates that it's on.
	//
	// Definitely get's it's own toggle in the app in case I hate it

	apply_brightness();

	// Render the current debug value as a dot
	render_debug_value(t_now_ms);  // (leds.h)
	
	// This value decays itself non linearly toward zero all the time, 
	// *really* slowing down the LPF when it's set to 1.0.
	// This is a super hacky way to fake a true fade transition between modes
	lpf_drag *= 0.995;
	
	// Apply a low pass filter to every color channel of every pixel on every frame
	// at hundreds of frames per second
	// 
	// To anyone who reads this: hobbyist microcontrollers are fucking insane now.
	// When I got into all this in 2012, I had a 16MHz single core AVR
	// 
	// The DMA and SIMD-style stuff inside the ESP32-S3 is some pretty crazy shit.
	float lpf_cutoff_frequency = 0.5 + (1.0-configuration.melt)*9.5;
	lpf_cutoff_frequency = lpf_cutoff_frequency * (1.0 - lpf_drag) + 0.5 * lpf_drag;
	apply_image_lpf(lpf_cutoff_frequency);

	// Quantize the image buffer with dithering, 
	// output to the 8-bit LED strand
	transmit_leds();

	// Update the FPS_GPU variable
	watch_gpu_fps(t_now_us);  // (system.h)

	uint32_t t_end_cycles = ESP.getCycleCount();
	volatile uint32_t gpu_total_cycles = t_end_cycles - t_start_cycles;
}