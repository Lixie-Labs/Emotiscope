let configuration = {};
let modes = [];
let sliders = [];
let toggles = [];

// TODO: Hue-related sliders should literally depict their hue settings
//   For example, the "hue" slider should fade to the currently selected color when dragged,
//   and the "hue range" slider should show a relevant gradient based on the position of
//   the "hue" slider.

function render_modes(){
	let mode_bin = document.getElementById("mode_bin");
	mode_bin.innerHTML = "";
	for(let i in modes){
		let mode_name = modes[i];
		let mode_button = `<div class="mode_button" onclick="set_mode('${mode_name}'); hide_page('page_modes');">${mode_name}</div>`;
		mode_bin.innerHTML += mode_button;
	}

	let current_mode = document.getElementById("current_mode");
	let current_mode_name = modes[configuration.current_mode];
	console.log("CURRENT MODE: "+current_mode_name);
	current_mode.innerHTML = current_mode_name;
}

// TODO: Make setting gallery snap magnetically into place when scrolling completes to keep sliders/toggles centered
function render_sliders(){
	let slider_container = document.getElementById("setting_container");
	slider_container.innerHTML = "";
	for(let i in sliders){
		let slider_name = sliders[i].name;
		let slider_min = sliders[i].min;
		let slider_max = sliders[i].max;
		let slider_step = sliders[i].step;
		let slider_value = configuration[slider_name];

		slider_container.innerHTML += `<span class="slider"><div class="slider_label" onclick="show_setting_information('${slider_name}');">${slider_name.toUpperCase().replace("_"," ")}</div><div class="slider_track" id="${slider_name}"></div></span>`;
		//slider_container.innerHTML += `<span class="slider"><div class="slider_label">${slider_name.toUpperCase()}</div><div class="slider_track" id="${slider_name}"></div></span>`;
	}
}

function render_toggles(){
	let toggle_container = document.getElementById("setting_container");
	let out_html = "";
	for(let i in toggles){
		let toggle_name = toggles[i].name;
		let toggle_value = configuration[toggle_name];

		out_html += `<span class="toggle">`;
		out_html += 	`<div class="toggle_label" onclick="show_setting_information('${toggle_name}');">`;
		out_html += 		`${toggle_name.toUpperCase().replace("_"," ")}`;
		out_html +=		`</div>`;
		out_html +=		`<div class="toggle_track" id="${toggle_name}"></div>`;
		out_html +=		`<div class="toggle_handle" id="${toggle_name}_handle"></div>`;
		out_html += `</span>`;
	}
	toggle_container.innerHTML += out_html;
}

function render_controls(){
	render_modes();

	render_sliders();
	render_toggles();

	set_sliders();
	set_toggles();

	track_sliders();
	track_toggles();
}

/*
let toggle_state = false;
function switch_toggle(){
	toggle_state = !toggle_state;
	set_toggle_state("mirror_mode", toggle_state);
}

setInterval(switch_toggle, 1000);
*/