#define MAX_COMMAND_LENGTH (64)

struct command {
	char command[MAX_COMMAND_LENGTH];
	uint8_t origin_client_slot;
};

struct freq {
	float target_freq;
	float coeff;
	float window_step;
	float magnitude;
	float magnitude_full_scale;
	float magnitude_last;
	float novelty;
	uint16_t block_size;
};

struct CRGBF {	// A bit like FastLED with floating point color channels that
				// get quantized to 8 bits with dithering later
	float r;
	float g;
	float b;
};

struct fx_dot {	 // Used to draw dots with subpixel precision and motion blur
	float position = 0.5;
	float position_past = 0.5;
};

struct lightshow_mode {
	char name[32];
	void (*draw)();
};

struct slider {
	char name[32];
	float slider_min;
	float slider_max;
	float slider_step;
};

struct toggle {
	char name[32];
};

struct menu_toggle {
	char name[32];
};

struct profiler_function {
	char name[32];
	uint32_t cycles_total;
	uint32_t hits;
};

struct tempo {
	float target_tempo_hz;
	float coeff;
	float sine;
	float cosine;
	float window_step;
	float phase;
	float phase_target;
	float phase_radians_per_reference_frame;
	float beat;
	float magnitude;
	float magnitude_full_scale;
	uint32_t block_size;
};

struct websocket_client {
	int socket;
	uint32_t last_ping;
};

struct CRGB8 {
	uint8_t g;
	uint8_t r;
	uint8_t b;
};

// CONFIGURATION STRUCTS OVER TIME: ---------------------------------------------------------------------------

// Current type: (the CONFIGURATION_TYPE number in configuration.h)
struct config {
	int32_t type; // Indirectly decribes the current datatypes of this struct (and their order in physical memory) to a parser

	float brightness;
	float softness; 
	float hue;
	float incandescent_filter;
	float hue_range;
	float speed;
	float saturation;
	float background;
	int32_t current_mode;
	bool mirror_mode;
	bool screensaver;
	bool temporal_dithering;
};

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// PAST VERSIONS BELOW, DON'T USE THESE: ----------------------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
// Configuration Type 1: Firmware 1.0.0
struct config_type_1 {
	int32_t type; // Indirectly decribes the current datatypes of this struct (and their order in physical memory) to a parser

	float brightness;
	float softness; 
	float hue;
	float incandescent_filter;
	float hue_range;
	float speed;
	float saturation;
	float background;
	int32_t current_mode;
	bool mirror_mode;
	bool screensaver;
	bool temporal_dithering;
};