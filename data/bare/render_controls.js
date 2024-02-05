let configuration = {};
let modes = [];
let sliders = [];
let toggles = [];

function render_modes(){
	let mode_bin = document.getElementById("mode_bin");
	mode_bin.innerHTML = "";
	for(let i in modes){
		let mode_name = modes[i];
		let mode_button = `<button onclick="set_mode('${mode_name}')">${mode_name}</button><br>`;
		mode_bin.innerHTML += mode_button;
	}
}

function render_sliders(){
	let slider_bin = document.getElementById("slider_bin");
	slider_bin.innerHTML = "";
	for(let i in sliders){
		let slider_name = sliders[i].name;
		let slider_min = sliders[i].min;
		let slider_max = sliders[i].max;
		let slider_step = sliders[i].step;
		let slider_value = configuration[slider_name];

		slider_bin.innerHTML += `<div class="slider_label">${slider_name}</div>`
		slider_bin.innerHTML += `<input type="range" class="slider" id="${slider_name}" oninput="send_slider_change('${slider_name}');" min="${slider_min}" max="${slider_max}" step="${slider_step}" value="${slider_value}"></input>`;
	}
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
	render_toggles();
}