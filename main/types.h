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
	ui_type_menu_toggle
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
} config;