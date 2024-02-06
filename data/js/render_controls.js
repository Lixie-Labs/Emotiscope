let configuration = {};
let modes = [];
let sliders = [];
let toggles = [];

// TODO: Add UI for toggles to setting gallery

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

function render_sliders(){
	let slider_container = document.getElementById("slider_container");
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

	set_sliders();
	track_sliders();
}

function render_toggles(){
	let toggle_bin = document.getElementById("toggle_bin");
	toggle_bin.innerHTML = "";
	for(let i in toggles){
		let toggle_name = toggles[i].name;
		let toggle_state = Boolean(configuration[toggle_name]);

		toggle_bin.innerHTML += `<div class="toggle_label">${toggle_name}</div>`
		toggle_bin.innerHTML += `<input type="checkbox" class="toggle" id="${toggle_name}" oninput="send_toggle_change('${toggle_name}');"/>`;

		document.getElementById(toggle_name).checked = toggle_state;
	}
}

function render_controls(){
	render_modes();
	render_sliders();
	//render_toggles();
}