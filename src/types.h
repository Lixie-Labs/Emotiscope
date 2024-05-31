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

struct websocket_client {
	int socket;
	uint32_t last_proof_of_life;
};

// union that stores the value of a config item
union config_value {
	uint32_t   u32;
	int32_t    i32;
	float      f32;
};

// enum that stores the type of a config item
enum type_t {
	u32,
	i32,
	f32
};

// enum that stores the UI type of a config item
enum ui_type_t {
	ui_type_none,
	ui_type_slider,
	ui_type_toggle,
	ui_type_menu_toggle
};

struct config_item {
	// Strings
	char name[16];
	char pretty_name[32];
	char type_string[16];
	char ui_type_string[16];

	// Enums
	type_t type;
	ui_type_t ui_type;

	// Raw Value
	config_value value;
};

// Stores all of Emotiscope's configurable settings
struct config {
	config_item brightness;
	config_item softness; 
	config_item color;
	config_item warmth;
	config_item color_range;
	config_item speed;
	config_item saturation;
	config_item background;
	config_item current_mode;
	config_item mirror_mode;
	config_item screensaver;
	config_item temporal_dithering;
	config_item auto_color_cycle;
	config_item reverse_color_range;
	config_item blur;
	config_item show_ui;
};