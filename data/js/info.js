let information = {
	"brightness"          : "The brightness of your Emotiscope's LED display.",
	"softness"            : "How quickly your Emotiscope's display reacts to changes; this can be used to reduce flickering effects at the cost of reaction time.",
	"speed"               : "How quickly things on screen can move",
	"color"               : "The base color of the current mode.",
	"color_range"         : "How much the color can vary from the base color.",
	"saturation"          : "The intensity of the color of the display.",
	"warmth"              : "Applies a filter which limits the brightness of blue colors, similar to the look of incandenscent light bulbs that were tinted by a colored film.",
	"background"          : "The intensity of the background color.",
	"mirror_mode"         : "Scales the display to 1/2 size and mirrors it to be symmetrical",
	"reverse_color_range" : "Reverses the direction of the color range, meaning that if your base color is red, the color range extends towards blue instead of green.",
	"auto_color_cycle"    : "Automatically cycles the base color to the music",
};

function show_setting_information(setting_name){
	setting_name = setting_name.toLowerCase();
	setting_name = setting_name.replace(" ", "_");

	let title = document.getElementById("info_panel_title");
	let description = document.getElementById("info_panel_description");

	let title_name = setting_name.toUpperCase();
	title_name = title_name.replace("_", " ");

	title.innerHTML = title_name;
	description.innerHTML = information[setting_name];

	let panel_div = document.getElementById("info_panel");
	panel_div.style.opacity = 1.0;
	panel_div.style.pointerEvents = "all";
}

function hide_setting_information(){
	let panel_div = document.getElementById("info_panel");
	panel_div.style.opacity = 0.0;
	panel_div.style.pointerEvents = "none";
}