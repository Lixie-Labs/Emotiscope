void draw_metronome() {
	static uint32_t iter = 0;
	iter++;

	for (uint16_t tempo_bin = 0; tempo_bin < NUM_TEMPI; tempo_bin++) {
		float progress = float(tempo_bin) / NUM_TEMPI;
		float tempi_magnitude = tempi_smooth[tempo_bin];

		float contribution = (tempi_magnitude / tempi_power_sum) * tempi_magnitude;

		if(contribution >= 0.00001){
			float sine = sin( tempi[tempo_bin].phase + (PI*0.5) );
			//sine *= 2.0;

			if(sine > 1.0){ sine = 1.0; }
			else if(sine < -1.0){ sine = -1.0; }

			float metronome_width;
			if(configuration.mirror_mode == true){
				metronome_width = 0.5;
			}
			else{
				metronome_width = 1.0;
			}

			float dot_pos = clip_float( sine * (0.5*(sqrt((contribution))) * metronome_width) + 0.5 );

			float opacity = clip_float(contribution*2.0);

			CRGBF dot_color = hsv(
				get_color_range_hue(progress),
				configuration.saturation,
				1.0
			);

			if(configuration.mirror_mode == true){
				dot_pos -= 0.25;
			}

			draw_dot(leds, NUM_RESERVED_DOTS + tempo_bin * 2 + 0, dot_color, dot_pos, opacity);

			if(configuration.mirror_mode == true){
				draw_dot(leds, NUM_RESERVED_DOTS + tempo_bin * 2 + 1, dot_color, 1.0 - dot_pos, opacity);
			}
		}
		else{
			// Put inactive dots in the middle
			fx_dots[NUM_RESERVED_DOTS + tempo_bin * 2 + 0].position = 0.5;

			if(configuration.mirror_mode == true){
				fx_dots[NUM_RESERVED_DOTS + tempo_bin * 2 + 0].position = 0.25;
				fx_dots[NUM_RESERVED_DOTS + tempo_bin * 2 + 1].position = 0.75;
			}
		}
	}
}