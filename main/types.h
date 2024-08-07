// enum that stores three light mode types: active, neutral, system
// Active: reacts to sound
// Inactive: does not react to sound
// System: used for system animations
typedef enum {
	LIGHT_MODE_TYPE_ACTIVE,
	LIGHT_MODE_TYPE_INACTIVE,
	LIGHT_MODE_TYPE_SYSTEM
} light_mode_type_t;

typedef enum {
	SELF_TEST_INACTIVE,
	SELF_TEST_STEP_START,
	SELF_TEST_STEP_LED,
	SELF_TEST_STEP_AUDIO,
	SELF_TEST_STEP_TOUCH,
	SELF_TEST_STEP_COMPLETE
} self_test_steps_t;

// Light Modes are stored in a struct with a name, type, and draw function
typedef struct {
	char name[32]; // ............ What the mode is called
	light_mode_type_t type; // ... What type of mode it is
	void (*draw)(); // ........... Function pointer to how it's drawn
} light_mode;


// Used for the Goertzel algorithm to detect frequencies in the audio
typedef struct {
	float target_freq; // ............ The frequency to detect
	float coeff; // .................. The coefficient used in the algorithm
	float window_step; // ............ The step size of the window
	float magnitude; // .............. The magnitude of the frequency
	float magnitude_full_scale; // ... The magnitude of the frequency at full scale
	float magnitude_last; // ......... The last magnitude of the frequency
	float novelty; // ................ The novelty of the frequency (spectral flux)
	uint16_t block_size; // .......... The size of the block
} freq;

typedef enum {
	UI_1, UI_2, UI_3, UI_4, UI_5, UI_6, UI_7, UI_8, UI_9, UI_10,
	UI_NEEDLE,
	SLEEP_1, SLEEP_2,
	SCREENSAVER_1, SCREENSAVER_2, SCREENSAVER_3, SCREENSAVER_4,
    NUM_RESERVED_DOTS
} reserved_dots_t;

// This is an equivalent to CRGB from FastLED
typedef struct {
	uint8_t g;
	uint8_t r;
	uint8_t b;
} CRGB8;

// CRGBFs are like CRGB8s, but with floating point color channels that
// get quantized to 8 bits with dithering later on in the pipeline
//
// They can also contain values higher than 1.0, which are HDR tonemapped before quantization
typedef struct {	
	float r;
	float g;
	float b;
} CRGBF;

// Used to draw dots with subpixel precision and motion blur
typedef struct {
	float position;
	float position_past;
} fx_dot;

typedef struct {
    float x, y;
} vec2;

typedef struct {
    unsigned int x, y;
} uvec2;

// A tempo-based version of the Goertzel struct above
typedef struct {
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
} tempo;

// Stores the state of capacitive touch pins
typedef struct {
	uint8_t pin;
	uint32_t touch_start;
	uint32_t touch_end;
	bool touch_active;
	bool hold_active;

	float touch_value;
	float ambient_threshold;
	float touch_threshold;
	float touch_history[50]; // 5 seconds at 10 FPS
} touch_pin;

typedef enum touch_position{
	TOUCH_LEFT = 0,
	TOUCH_CENTER = 1,
	TOUCH_RIGHT = 2
} touch_position;

typedef enum {
	UI_SHOW_EVENT,
    UI_NEEDLE_EVENT,
    UI_HUE_EVENT,
} ui_update_event;

// enum that stores the type of a config item
typedef enum {
	COLOR_MODE_GRADIENT,
	COLOR_MODE_PERLIN
} color_mode_t;

// This is for profiling functions
typedef struct {
	char name[16];
	uint32_t hits[2];
} profiled_function;

// union that stores the value of a config item
typedef union {
	uint32_t   u32;
	int32_t    i32;
	float      f32;
} config_value;

// enum that stores the type of a config item
typedef enum {
	u32t,
	i32t,
	f32t
} type_t;

// enum that stores the UI type of a config item
typedef enum {
	ui_type_none,
	ui_type_slider,
	ui_type_toggle,
	ui_type_menu_toggle,
	ui_type_mode,
} ui_type_t;

typedef struct {
	// Strings
	char name[16];
	char pretty_name[32];
	char type_string[16];
	char ui_type_string[16];

	// Enums
	type_t type;
	ui_type_t ui_type;

	config_value value;
	config_value default_value;
} config_item;

// Stores all of Emotiscope's configurable settings
typedef struct{
	// Sliders
	config_item brightness;
	config_item softness; 
	config_item color;
	config_item saturation;
	config_item color_range;
	config_item warmth;
	config_item background;
	config_item blur;

	// Toggles
	config_item mirror_mode;
	config_item color_mode;
	config_item auto_color_cycle;
	config_item reverse_color_range;	

	// Mode
	config_item current_mode;

	// Menu Toggles
	config_item screensaver;
	config_item temporal_dithering;
	config_item show_ui;

	// Deprecated
	config_item speed;
} config;