function set_toggle_state(toggle_name, toggle_state){
	if(toggle_state == true){
		document.getElementById(toggle_name+"_handle").style.top = "calc(-100% + 36px)";
		document.getElementById(toggle_name+"_handle").style.backgroundColor = "#5897ce";
		document.getElementById(toggle_name).style.border = "2px solid #5897ce";
	}
	else if(toggle_state == false){
		let offset = document.getElementById(toggle_name).offsetHeight - 70;
		document.getElementById(toggle_name+"_handle").style.top = "calc(-100% + 36px + "+offset+"px)";
		document.getElementById(toggle_name+"_handle").style.backgroundColor = "#5d5d5d";
		document.getElementById(toggle_name).style.border = "2px solid #5d5d5d";
	}
}

function set_toggles(){
	// Find all elements with the class 'toggle_track'
	var toggle_tracks = document.querySelectorAll('.toggle_track');

	// Iterate over each 'toggle_track' element
	toggle_tracks.forEach(function(toggle_track) {
		// Get the 'id' attribute of the current element
		var id = toggle_track.getAttribute('id');
		console.log("ID: "+id);
		console.log(configuration);

		try{
			// Use the 'id' to access the corresponding value in the 'configuration' JSON
			var value = configuration[id];
			console.log('Value for', id, ':', value);

			for(let i in toggles){
				let toggle = toggles[i];
				let toggle_name = toggle.name;

				if(toggle_name == id){
					set_toggle_state(id, value);
				}
			}    
		} catch(e) {
			// Log a message if there's no configuration for the current 'id'
			console.log('No value found for', id);
		}
	});
}

function track_toggles() {
    const touch_start_data_toggles = new Map(); // To store initial data for each touch

    function start_tracking_toggle(event) {
		console.log("snapping off");
		magnetic_snapping_enabled = false;

        Array.from(event.touches).forEach(touch => {
            // Store initial touch positions
            touch_start_data_toggles.set(touch.identifier, {
                start_x: touch.clientX,
                start_y: touch.clientY,
                target_div: touch.target.closest('.toggle_track, .toggle_handle'),
                tracking_allowed: undefined // We haven't determined the direction yet
            });
        });
    }

    function track_movement_toggle(event) {
        Array.from(event.touches).forEach(touch => {
            const data = touch_start_data_toggles.get(touch.identifier);

            if (data && data.target_div) {
                const delta_x = Math.abs(touch.clientX - data.start_x);
                const delta_y = Math.abs(touch.clientY - data.start_y);

                // Determine the direction if not done yet and the touch has moved more than 10 pixels
                if (data.tracking_allowed === undefined && (delta_x > 10 || delta_y > 10)) {
                    // Allow tracking for more vertical movement; otherwise, keep default behavior
                    data.tracking_allowed = delta_y > delta_x;
                }

                if (data.tracking_allowed) {
                    event.preventDefault(); // Prevent default behavior for vertical drags

					var id = data.target_div.getAttribute('id').replace("_handle", "");

                    // Tracking logic here
                    if (delta_y > 0) { // Ensure there's some vertical movement to track
                        let distance_moved = data.start_y - touch.clientY;

						for(let i in toggles){
							let toggle = toggles[i];
							let toggle_name = toggle.name;

							if(toggle_name == id){
								let resulting_value = configuration[id];

								if(distance_moved <= -20){
									set_toggle_state(toggle.name, false);
									resulting_value = 0;
								}
								else if(distance_moved >= 20){
									set_toggle_state(toggle.name, true);
									resulting_value = 1;
								}

								if(configuration[id] != resulting_value){
									configuration[id] = resulting_value;
									trigger_vibration(10);
									transmit(`set|${id}|${resulting_value}`);
								}

								//let percentage = ((resulting_value-slider_min) / (slider_max-slider_min)) * 100.0;

								//data.target_div.style.backgroundImage = `linear-gradient(to top, var(--secondary) 0%, var(--secondary) ${percentage}%, transparent 0%, transparent 100%)`;
							}
						}
                    }
                }
            }
        });
    }

    function stop_tracking_toggle(event) {
		console.log(configuration);

		console.log("snapping on");
		magnetic_snapping_enabled = true;

        Array.from(event.changedTouches).forEach(touch => {
            touch_start_data_toggles.delete(touch.identifier);
        });
    }

    // Apply event listeners to all toggle track divs
    document.querySelectorAll('.toggle_track').forEach(toggle_track => {
        toggle_track.addEventListener('touchstart', start_tracking_toggle, {passive: false});
    });
	document.querySelectorAll('.toggle_handle').forEach(toggle_handle => {
        toggle_handle.addEventListener('touchstart', start_tracking_toggle, {passive: false});
    });

    // Add move and end listeners to document to ensure capture even outside divs
    document.addEventListener('touchmove', track_movement_toggle, {passive: false});
    document.addEventListener('touchend', stop_tracking_toggle);
}