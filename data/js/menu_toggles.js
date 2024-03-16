function set_menu_toggle_state(toggle_name, toggle_state){
	console.log("TOGGLE: "+toggle_name);
	if(toggle_state == true){
		document.getElementById(toggle_name).classList.remove("menu_item_toggle_off");
		document.getElementById(toggle_name).classList.add("menu_item_toggle_on");
		document.getElementById(toggle_name).innerHTML = "ON";
		transmit(`set|${toggle_name}|1`);
	}
	else{
		document.getElementById(toggle_name).classList.remove("menu_item_toggle_on");
		document.getElementById(toggle_name).classList.add("menu_item_toggle_off");
		document.getElementById(toggle_name).innerHTML = "OFF";
		transmit(`set|${toggle_name}|0`);
	}

	configuration[toggle_name] = !configuration[toggle_name];

	render_menu_toggles();
}

function set_menu_toggles(){
	// Find all elements with the class 'toggle_track'
	/*
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
	*/
}