let ws;
let device_ip = "192.168.1.49";

let auto_response_table = {
	"welcome":"get|config",
	"config_ready":"get|modes",
	"modes_ready":"get|sliders",
	"sliders_ready":"get|toggles",
	"mode_selected":"get|config",
};

function set_locked_state(locked_state){
	let dimmer = document.getElementById("dimmer");
	if(locked_state == true){
		dimmer.style.opacity = 1.0;
		dimmer.style.pointerEvents = "all";
		//document.getElementById("device_preview").innerHTML = "LOCK";
	}
	else{
		dimmer.style.opacity = 0.0;
		dimmer.style.pointerEvents = "none";
		//document.getElementById("device_preview").innerHTML = "UNLOCK";
	}
}

function attempt_auto_response(message){
	let success = false;
	try{
		let reply = auto_response_table[message];
		if(reply != undefined){
			console.log(`Auto-reply for message ${message} is: ${reply}`);
			transmit( reply );
			success = true;
		}
	}
	catch(e){
		console.log(e);
	}

	return success;
}

function sync_data_from_device(){
	transmit("get|config"); // Triggers chain of data sync commands
}

function parse_message(message){
	if( attempt_auto_response(message) == false){
		// parse reply contents
		let command_data = message.split("|");
		let command_type = command_data[0];

		if(command_type == "clear_config"){
			// Clear client-side config JSON
			configuration = {};

			set_locked_state(true);
		}
		else if(command_type == "new_config"){
			// Append new config key to client-side config JSON
			let config_key_name  = command_data[1];
			let config_data_type = command_data[2];
			let config_value_raw = command_data[3];
			let config_value;

			if(config_data_type == "string"){
				config_value = config_value_raw;
			}
			else if(config_data_type == "float"){
				config_value = parseFloat(config_value_raw);
			}
			else if(config_data_type == "int"){
				config_value = parseInt(config_value_raw);
			}
			else{
				console.log(`UNRECOGNIZED CONFIG DATA TYPE: ${config_data_type}`);
			}

			configuration[config_key_name] = config_value;
		}
		else if(command_type == "clear_modes"){
			modes = [];
		}
		else if(command_type == "new_mode"){
			let mode_name = command_data[1];
			modes.push(mode_name);
		}
		else if(command_type == "clear_sliders"){
			sliders = [];
		}
		else if(command_type == "new_slider"){
			let slider_name = command_data[1];
			let slider_min  = parseFloat(command_data[2]);
			let slider_max  = parseFloat(command_data[3]);
			let slider_step = parseFloat(command_data[4]);

			sliders.push({
				"name":slider_name,
				"min":slider_min,
				"max":slider_max,
				"step":slider_step
			});
		}
		else if(command_type == "clear_toggles"){
			toggles = [];
		}
		else if(command_type == "new_toggle"){
			let toggle_name = command_data[1];

			toggles.push({
				"name":toggle_name
			});
		}
		else if(command_type == "toggles_ready"){
			console.log("DATA SYNC COMPLETE!");

			render_controls();
			//tint_svg_images();
			set_locked_state(false);
		}
		else{
			console.log(`Unrecognized command type: ${command_type}`);
		}
	}
}

function set_mode(mode_name){
	transmit(`set|mode|${mode_name}`);
}

function send_slider_change(slider_name){
	let new_value = document.getElementById(slider_name).value;
	transmit(`set|${slider_name}|${new_value}`);
}

function send_toggle_change(toggle_name){
	let new_state = +(document.getElementById(toggle_name).checked);
	transmit(`set|${toggle_name}|${new_state}`);
}

function transmit(message){
	console.log(`TX: ${message}`);
	//document.getElementById("device_preview").innerHTML = message;
	ws.send(message);
}

function open_websockets_connection_to_device(){
	ws = new WebSocket("ws://"+device_ip+":80/ws");

	ws.onopen = function(e) {
		console.log("[open] Connection established");
	};

	ws.onmessage = function(event) {
		console.log(`RX: ${event.data}`);
		//document.getElementById("device_preview").innerHTML = event.data;
		parse_message(event.data);
	};

	ws.onclose = function(event) {
		if (event.wasClean) {
			console.log(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
		} else {
			// e.g. server process killed or network down
			// event.code is usually 1006 in this case
			console.log('[close] Connection died');
		}
	};

	ws.onerror = function(error) {
		console.log(`[error]`);
	};
}

open_websockets_connection_to_device();