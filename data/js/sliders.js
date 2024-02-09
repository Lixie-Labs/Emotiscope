var temporary_configuration = {};

function set_sliders(){
	// Find all elements with the class 'slider_track'
	var slider_tracks = document.querySelectorAll('.slider_track');

	// Iterate over each 'slider_track' element
	slider_tracks.forEach(function(slider_track) {
		// Get the 'id' attribute of the current element
		var id = slider_track.getAttribute('id');

		// Check if there's a configuration for the current 'id'
		try{
			// Use the 'id' to access the corresponding value in the 'configuration' JSON
			var value = configuration[id];
			// Log the configuration to the console
			console.log('Value for', id, ':', value);

			for(let i in sliders){
				let slider = sliders[i];
				let slider_name = slider.name;

				if(slider_name == id){
					let slider_min = slider.min;
					let slider_max = slider.max;
					
					let percentage = ((value-slider_min) / (slider_max-slider_min)) * 100.0;

					slider_track.style.backgroundImage = `linear-gradient(to top, var(--secondary) 0%, var(--secondary) ${percentage}%, transparent 0%, transparent 100%)`;
				}
			}    
		}
		catch(e) {
			// Log a message if there's no configuration for the current 'id'
			console.log('No value found for', id);
		}
	});
}

function track_sliders() {
    const touch_start_data = new Map(); // To store initial data for each touch

    function start_tracking(event) {
		temporary_configuration = JSON.parse(JSON.stringify(configuration));

        Array.from(event.touches).forEach(touch => {
            // Store initial touch positions
            touch_start_data.set(touch.identifier, {
                start_x: touch.clientX,
                start_y: touch.clientY,
                target_div: touch.target.closest('.slider_track'),
                tracking_allowed: undefined // We haven't determined the direction yet
            });
        });
    }

    function track_movement(event) {
        Array.from(event.touches).forEach(touch => {
            const data = touch_start_data.get(touch.identifier);

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

					var id = data.target_div.getAttribute('id');

                    // Tracking logic here
                    if (delta_y > 0) { // Ensure there's some vertical movement to track
						let distance_moved = data.start_y - touch.clientY;
                        let ratio = Math.min(1.0, Math.max(-1.0, (distance_moved / data.target_div.offsetHeight)));
                        let div_name = data.target_div.getAttribute('name') || data.target_div.id;

						for(let i in sliders){
							let slider = sliders[i];
							let slider_name = slider.name;

							if(slider_name == id){
								let slider_min = slider.min;
								let slider_max = slider.max;
								let range = (slider_max-slider_min);
								let add_amount = ratio*range;

								let resulting_value = Math.min(slider_max, Math.max(slider_min, temporary_configuration[id] + add_amount));

								if(configuration[id] != resulting_value){
									configuration[id] = resulting_value;
									transmit(`set|${id}|${truncate_float(resulting_value, 3)}`);
								}

								let percentage = ((resulting_value-slider_min) / (slider_max-slider_min)) * 100.0;

								data.target_div.style.backgroundImage = `linear-gradient(to top, var(--secondary) 0%, var(--secondary) ${percentage}%, transparent 0%, transparent 100%)`;
							}
						}
					}
                }
            }
        });
    }

    function stop_tracking(event) {
		console.log(configuration);

        Array.from(event.changedTouches).forEach(touch => {
            touch_start_data.delete(touch.identifier);
        });
    }

    // Apply event listeners to all slider track divs
    document.querySelectorAll('.slider_track').forEach(slider_track => {
        slider_track.addEventListener('touchstart', start_tracking, {passive: false});
    });

    // Add move and end listeners to document to ensure capture even outside divs
    document.addEventListener('touchmove', track_movement, {passive: false});
    document.addEventListener('touchend', stop_tracking);
}