void draw_spectronome(){
	start_profile(__COUNTER__, __func__);
	// Draw spectrograph
	draw_spectrum();

	// Darken it by how much confidence I have in the current tempo guess
	scale_CRGBF_array_by_constant(leds, (1.0 - sqrt(sqrt(tempo_confidence)))*0.85 + 0.15, NUM_LEDS);

	draw_metronome();
	end_profile();
}