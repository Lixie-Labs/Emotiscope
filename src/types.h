/*
-----------------------------------------------------------------------------
--- EMOTISCOPE ENGINE -------------------------------------------------------

	types.h
		- Custom data types are defined here
   
-----------------------------------------------------------------------------
*/

// enum that stores three light mode types: active, neutral, system
// Active: reacts to sound
// Inactive: does not react to sound
// System: used for system animations
enum light_mode_type_t {
	LIGHT_MODE_TYPE_ACTIVE,
	LIGHT_MODE_TYPE_INACTIVE,
	LIGHT_MODE_TYPE_SYSTEM
};

enum self_test_steps_t {
	SELF_TEST_INACTIVE,
	SELF_TEST_STEP_START,
	SELF_TEST_STEP_LED,
	SELF_TEST_STEP_AUDIO,
	SELF_TEST_STEP_TOUCH,
	SELF_TEST_STEP_COMPLETE
};

// Light Modes are stored in a struct with a name, type, and draw function
struct light_mode {
	char name[32]; // ............ What the mode is called
	light_mode_type_t type; // ... What type of mode it is
	void (*draw)(); // ........... Function pointer to how it's drawn
};

// A command is a string of characters that is sent from the web app to
// Emotiscope, or vice-versa. It also stores the origin client slot.
struct command {
	char command[MAX_COMMAND_LENGTH];
	uint8_t origin_client_slot;
};

// Used for the Goertzel algorithm to detect frequencies in the audio
struct freq {
	float target_freq; // ............ The frequency to detect
	float coeff; // .................. The coefficient used in the algorithm
	float window_step; // ............ The step size of the window
	float magnitude; // .............. The magnitude of the frequency
	float magnitude_full_scale; // ... The magnitude of the frequency at full scale
	float magnitude_last; // ......... The last magnitude of the frequency
	float novelty; // ................ The novelty of the frequency (spectral flux)
	uint16_t block_size; // .......... The size of the block
};

// This is equivalent to CRGB from FastLED
struct CRGB8 {
	uint8_t g;
	uint8_t r;
	uint8_t b;
};

// CRGBFs are like CRGB8s, but with floating point color channels that
// get quantized to 8 bits with dithering later on in the pipeline
struct CRGBF {	
	float r;
	float g;
	float b;
};

// Used to draw dots with subpixel precision and motion blur
struct fx_dot {
	float position = 0.5;
	float position_past = 0.5;
};

// Used to define Sliders in the web app
struct slider {
	char name[32];
	float slider_min;
	float slider_max;
	float slider_step;
};

// Used to define Toggles in the web app
struct toggle {
	char name[32];
};

// Used to define Menu Toggles in the web app
struct menu_toggle {
	char name[32];
};

// Used to define Menu Dropdowns in the web app
/*
struct menu_dropdown {
	char name[32];
	char options[32][32];
};
*/

// Used to store the state of profiled functions
struct profiler_function {
	char name[32];
	uint32_t cycles_total;
	uint32_t hits;
};

// A tempo-based version of the Goertzel struct above
struct tempo {
	float target_tempo_hz; 
	float coeff;
	float sine;
	float cosine;
	float window_step;
	float phase;
	float phase_target;
	bool  phase_inverted;
	float phase_radians_per_reference_frame;
	float beat;
	float magnitude;
	float magnitude_full_scale;
	uint32_t block_size;
};

// Clients are like Players in the web app. Like a video game console, they
// are plugged into one of four sockets
struct websocket_client {
	int socket;
	uint32_t last_ping;
};

// Stores the state of capacitive touch pins
struct touch_pin {
	uint8_t pin;
	uint32_t touch_start;
	uint32_t touch_end;
	bool touch_active;
	bool hold_active;

	float touch_value;
	float ambient_threshold;
	float touch_threshold;
	float touch_history[50]; // 5 seconds at 10 FPS
};

// Stores all of Emotiscope's configurable settings
struct config {
	float brightness;
	float softness; 
	float color;
	float blue_filter;
	float color_range;
	float speed;
	float saturation;
	float background;
	int32_t current_mode;
	bool mirror_mode;
	bool screensaver;
	bool temporal_dithering;
	float vu_floor;
	bool auto_color_cycle;
	bool reverse_color_range;
};