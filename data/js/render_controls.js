let configuration = {};
let modes = [];
let sliders = [];
let toggles = [];

let magnetic_snapping_enabled = true;

// TODO: Hue-related sliders should literally depict their hue settings
//   For example, the "hue" slider should fade to the currently selected color when dragged,
//   and the "hue range" slider should show a relevant gradient based on the position of
//   the "hue" slider.

function trigger_vibration(length_ms) {
	console.log("~~~~~~~~~~~~~buzz~~~~~~~~~~~~~");
    // Check if the Vibrate API is supported in the navigator
    if ("vibrate" in navigator) {
        // Trigger vibration for the specified length
        navigator.vibrate([length_ms]);
        return true; // Indicate that the vibration was triggered
    } else {
        // Vibrate API not supported
        return false; // Indicate that the vibration could not be triggered
    }
}

function render_modes(){
	let mode_bin = document.getElementById("mode_bin");
	mode_bin.innerHTML = "";
	for(let i in modes){
		let mode_name = modes[i];
		let mode_button = `<div class="mode_button buzz" onclick="set_mode('${mode_name}'); hide_page('page_modes');">${mode_name}</div>`;
		mode_bin.innerHTML += mode_button;
	}

	let current_mode = document.getElementById("current_mode");
	let current_mode_name = modes[configuration.current_mode]; 
	console.log("CURRENT MODE: "+current_mode_name);
	current_mode.innerHTML = current_mode_name;
}

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
// TODO: Make the setting gallery snap magnetically into place when scrolling completes to keep sliders/toggles centered

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

function calculate_slider_distance() {
    // Get all elements with the class "slider"
    var sliders = document.getElementsByClassName("slider");

    // Ensure there are at least two sliders to measure the distance between
    if (sliders.length < 2) {
        console.error("Not enough sliders found to calculate the distance.");
        return 0; // Return 0 as there aren't enough sliders to measure distance
    }

    // Get bounding rectangle of the first slider
    var first_slider_rect = sliders[0].getBoundingClientRect();
    // Get bounding rectangle of the second slider
    var second_slider_rect = sliders[1].getBoundingClientRect();

    // Calculate the horizontal distance between the first two sliders
    // Note: This calculation assumes the sliders are aligned in some way (vertically or at least partially)
    // For a more robust solution, consider handling cases where sliders are not aligned vertically.
    var distance = Math.abs(second_slider_rect.left - first_slider_rect.left);

    // Return the calculated distance
    return distance;
}

function v_sync(callback) {
    // The function to be called repeatedly by requestAnimationFrame
    function frame() {
        // Execute the callback function passed to v_sync
        callback();

        // Request the next frame
        requestAnimationFrame(frame);
    }

    // Start the animation loop
    requestAnimationFrame(frame);
}

function init_vibration(){
	v_sync(function(){
		// Nothing
	});
}

function init_setting_gallery_snapping(){
	let nearest_click_point_last_frame = 0;
	v_sync(function(){
		
		// Do this every screen redraw
		var setting_gallery = document.getElementById('setting_gallery');
		var scroll_position = setting_gallery.scrollLeft;
		//console.log("SCROLL POSITION: "+scroll_position);

		// Calculate how far apart each snap point in the setting gallery is based on screen width
		let snap_interval = calculate_slider_distance();

		let magnetic_bounds = [];
		let click_points = [];
		let click_offset = (snap_interval / 2);
		let num_magnetic_bounds = (sliders.length + toggles.length) - 2; // Not counting edges
		let num_click_points = num_magnetic_bounds;
		for(let i = 0; i < num_magnetic_bounds; i++){
			magnetic_bounds.push(  snap_interval * i );
			click_points.push( snap_interval * i + click_offset );
		}

		let min_distance = 100000000;
		let nearest_magnetic_bound = -1;
		for(let i = 0; i < num_magnetic_bounds; i++){
			let absolute_distance = Math.abs(magnetic_bounds[i] - scroll_position);
			if(absolute_distance < min_distance){
				min_distance = absolute_distance;
				nearest_magnetic_bound = i;
			}
		}

		min_distance = 100000000;
		let nearest_click_point = -1;
		for(let i = 0; i < num_click_points; i++){
			let absolute_distance = Math.abs(click_points[i] - scroll_position);
			if(absolute_distance < min_distance){
				min_distance = absolute_distance;
				nearest_click_point = i;
			}
		}

		if(nearest_click_point != nearest_click_point_last_frame){
			trigger_vibration(5);
			nearest_click_point_last_frame = nearest_click_point;
		}

		let magnetic_pull = magnetic_bounds[nearest_magnetic_bound] - scroll_position; // 100 - 90 = 10
		//console.log(magnetic_pull);
			
		if(magnetic_snapping_enabled){
			setting_gallery.scrollLeft += magnetic_pull * 0.2;
		}
	});
}

function render_controls(){
	console.log("render_controls()");
	init_setting_gallery_snapping();
	render_modes();

	render_sliders();
	render_toggles();

	set_sliders();
	set_toggles();

	track_sliders();
	track_toggles();
}

function attach_buzz_listeners_to_element(element) {
    element.addEventListener('touchstart', function(){
		trigger_vibration(10);
		element.classList.add('buzz_pressed');
	});
    element.addEventListener('touchend', function(){
		trigger_vibration(10);
		element.classList.remove('buzz_pressed');
	});
    element.dataset.hasListeners = 'true'; // Mark the element to avoid attaching listeners again
}

function check_and_attach_buzz_listeners() {
    const buzz_elements = document.querySelectorAll('.buzz:not([data-has-listeners])');
    buzz_elements.forEach(attach_buzz_listeners_to_element);
}

let setting_gallery = document.getElementById("setting_gallery");
setting_gallery.addEventListener('touchstart', function(){
	trigger_vibration(5);
	console.log("snapping_off");
	magnetic_snapping_enabled = false;
});
setting_gallery.addEventListener('touchend', function(){
	trigger_vibration(5);
	console.log("snapping_on");
	magnetic_snapping_enabled = true;
});

/*
document.getRootNode().addEventListener('touchstart', function(){
	trigger_vibration(10);
});

document.getRootNode().addEventListener('touchend', function(){
	trigger_vibration(5);
});
*/

// Periodically check for new '.buzz' elements every 100ms
setInterval(check_and_attach_buzz_listeners, 100);