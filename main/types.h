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