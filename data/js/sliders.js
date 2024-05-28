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
			//console.log('Value for', id, ':', value);

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
			//console.log('No value found for', id);
		}
	});
}

function track_sliders() {
    const touch_start_data = new Map(); // To store initial data for each touch

    function start_tracking(event) {
		temporary_configuration = JSON.parse(JSON.stringify(configuration));

		// Check if event is a mouse event
		const isMouseEvent = event.touches === undefined;
 		const eventPoints = isMouseEvent ? [event] : (event.touches ? Array.from(event.touches) : []);

		eventPoints.forEach(point => {
			let id = point.target.closest('.slider_track').getAttribute('id');
			wstx(`slider_touch_start|${id}`);

			// Store initial touch positions
			touch_start_data.set(isMouseEvent ? 'mouse' : point.identifier, {
				start_x: point.clientX,
				start_y: point.clientY,
				target_div: point.target.closest('.slider_track'),
				tracking_allowed: undefined // We haven't determined the direction yet
			});
		});
	}

    function track_movement(event) {
		const isMouseEvent = event.touches === undefined;
 		const eventPoints = isMouseEvent ? [event] : (event.touches ? Array.from(event.touches) : []);

        eventPoints.forEach(point => {
            const data = touch_start_data.get(isMouseEvent ? 'mouse' : point.identifier);

            if (data && data.target_div) {
                const delta_x = Math.abs(point.clientX - data.start_x);
                const delta_y = Math.abs(point.clientY - data.start_y);

                // Determine the direction if not done yet and the touch has moved more than 10 pixels
                if (data.tracking_allowed === undefined && (delta_x > 10 || delta_y > 10)) {
                    // Allow tracking for more vertical movement; otherwise, keep default behavior
                    data.tracking_allowed = delta_y > delta_x;
                }

                if (data.tracking_allowed) {
                    // Prevent default behavior for dragging (scroll) only for touch events
					//if (!isMouseEvent) {
						event.preventDefault();
					//}

					var id = data.target_div.getAttribute('id');

                    // Tracking logic here
                    if (delta_y > 0) { // Ensure there's some vertical movement to track
						let distance_moved = data.start_y - point.clientY;
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
									wstx(`set|${id}|${truncate_float(resulting_value, 3)}`);
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
    const isMouseEvent = event.changedTouches === undefined;
    const eventPoints = isMouseEvent ? [event] : (event.changedTouches ? Array.from(event.changedTouches) : []);

    eventPoints.forEach(point => {
        const data = touch_start_data.get(isMouseEvent ? 'mouse' : point.identifier);

        if (data && data.target_div.classList.contains("slider_track")) {
            let id = data.target_div.getAttribute('id');
            wstx(`slider_touch_end|${id}`);
        }

        touch_start_data.delete(isMouseEvent ? 'mouse' : point.identifier);
    });
}

    // Apply event listeners to all slider track divs
    document.querySelectorAll('.slider_track').forEach(slider_track => {
        slider_track.addEventListener('touchstart', start_tracking, {passive: false});
		slider_track.addEventListener('mousedown', start_tracking, {passive: false});
    });

    // Add move and end listeners to document to ensure capture even outside divs
    document.addEventListener('touchmove', track_movement, {passive: false});
	document.addEventListener('mousemove', track_movement, {passive: false});

    document.addEventListener('touchend', stop_tracking, {passive: false});
	document.addEventListener('mouseup', stop_tracking, {passive: false});

	const draggable_container = document.getElementById('setting_gallery');
	let is_down = false;
	let start_x;
	let scroll_left;

	draggable_container.addEventListener('mousedown', (e) => {
		is_down = true;
		draggable_container.style.cursor = 'grabbing';
		start_x = e.pageX - draggable_container.offsetLeft;
		scroll_left = draggable_container.scrollLeft;
	});

	draggable_container.addEventListener('mouseleave', () => {
		is_down = false;
		draggable_container.style.cursor = 'grab';
	});

	draggable_container.addEventListener('mouseup', () => {
		is_down = false;
		draggable_container.style.cursor = 'grab';
	});

	draggable_container.addEventListener('mousemove', (e) => {
		if (!is_down) return;
		e.preventDefault();
		const x = e.pageX - draggable_container.offsetLeft;
		const walk = (x - start_x); // Adjust the speed and responsiveness of scrolling
		draggable_container.scrollLeft = scroll_left - walk;
	});
}