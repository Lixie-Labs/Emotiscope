float vu_level_smooth = 0.000001;

void draw_analog(){
	float mix_speed = 0.005 + 0.145*configuration.speed;

	vu_level_smooth = vu_level * mix_speed + vu_level_smooth*(1.0-mix_speed);
	float dot_pos = clip_float(vu_level_smooth);
	CRGBF dot_color = hsv(configuration.hue + configuration.hue_range*dot_pos, configuration.saturation, 1.0);

	if(configuration.mirror_mode == true){
		draw_dot(leds, NUM_RESERVED_DOTS+0, dot_color, 0.5 + (dot_pos* 0.5), 1.0);
		draw_dot(leds, NUM_RESERVED_DOTS+1, dot_color, 0.5 + (dot_pos*-0.5), 1.0);
	}
	else{
		draw_dot(leds, NUM_RESERVED_DOTS+0, dot_color, dot_pos, 1.0);
	}
}