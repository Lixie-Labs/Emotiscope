const MAX_PING_PONG_REPLY_TIME_MS = 4000;
const MAX_CONNECTION_TIME_MS = 3000;
const AUTO_RECONNECT = true;

let ws;
let device_ip;
let connection_start_time;
let connection_pending = false;
let last_ping_time;
let pong_pending = false;
let reconnecting = false;
let standby_mode = false;
let pongs_halted = false;

let touch_vals = [
	0,
	0,
	0
];

let touch_low = [
	0,
	0,
	0
];

let touch_high = [
	0,
	0,
	0
];

got_touch_vals = true;

let auto_response_table = {
	"welcome":"get|config",
	"config_ready":"get|modes",
	"modes_ready":"get|sliders",
	"sliders_ready":"get|toggles",
	"toggles_ready":"get|menu_toggles",
	"mode_selected":"get|config",
};

function check_connection_timeout(){
	if(connection_pending == true){
		if(performance.now() - connection_start_time >= MAX_CONNECTION_TIME_MS){
			console.log("COULDN'T CONNECT TO DEVICE WITHIN TIMEOUT");
			reconnect_websockets();
		}
	}
}

function ping_server(){
	ws.send("ping");
	last_ping_time = performance.now();
	pong_pending = true;
}

function check_pong_timeout(){
	if(pongs_halted == false){
		if(pong_pending == true){
			if(performance.now() - last_ping_time >= MAX_PING_PONG_REPLY_TIME_MS){
				console.log("NO PONG WITHIN TIMEOUT!");
				reconnect_websockets();
			}
		}
	}
}

function set_ui_locked_state(locked_state){
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

function start_noise_calibration(){
	set_ui_locked_state(true);
	transmit('noise_cal');
}

function start_debug_recording(){
	set_ui_locked_state(true);
	transmit('start_debug_recording');
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

			set_ui_locked_state(true);
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
		else if(command_type == "clear_menu_toggles"){
			menu_toggles = [];
		}
		else if(command_type == "new_menu_toggle"){
			let toggle_name = command_data[1];

			menu_toggles.push({
				"name":toggle_name
			});
		}
		else if(command_type == "menu_toggles_ready"){
			console.log("DATA SYNC COMPLETE!");
			ping_server();
			setInterval(check_pong_timeout, 100);
			render_controls();
			//tint_svg_images();
			set_ui_locked_state(false);
		}
		else if(command_type == "noise_cal_ready"){
			console.log("NOISE CAL COMPLETE!");
			hide_page('page_calibration');
			set_ui_locked_state(false);
		}
		else if(command_type == "debug_recording_ready"){
			console.log("DEBUG RECORDING COMPLETE!");
			hide_page('page_calibration');
			set_ui_locked_state(false);
		}
		else if(command_type == "fps_cpu"){
			let FPS = command_data[1];
			document.getElementById("CPU_FPS").innerHTML = `CPU FPS: ${FPS}`;
		}
		else if(command_type == "fps_gpu"){
			let FPS = command_data[1];
			document.getElementById("GPU_FPS").innerHTML = `GPU FPS: ${FPS}`;
		}
		else if(command_type == "heap"){
			let heap = command_data[1];
			document.getElementById("HEAP").innerHTML = `HEAP: ${heap}`;
		}
		else if(command_type == "pong"){
			pong_pending = false;
			setTimeout(function(){
				ping_server();
			}, MAX_PING_PONG_REPLY_TIME_MS / 2);
		}
		else if(command_type == "touch_vals"){
			touch_vals[0] = parseInt(command_data[1]);
			touch_vals[1] = parseInt(command_data[2]);
			touch_vals[2] = parseInt(command_data[3]);

			console.log(`TOUCH VALS: ${touch_vals}`);

			got_touch_vals = true;
		}
		else if(command_type == "update_available"){
			pongs_halted = true;
			show_alert(
				"UPDATE AVAILABLE",
				"An update is available for your Emotiscope!<br><br>Click below to download the latest firmware.",
				"UPDATE NOW",
				function(){
					hide_alert();
					transmit("perform_update");
				}
			);
		}
		else if(command_type == "no_updates"){
			show_alert(
				"ALREADY UP-TO-DATE",
				"Your Emotiscope is already running the latest firmware!<br><br>(But follow @lixielabs on social media to keep <em>yourself</em> up-to-date on new features and improvements that are coming!)",
				"DAMN, OK",
				function(){
					hide_alert();
				}
			);
		}
		else if(command_type == "ota_firmware_progress"){
			let progress = parseInt(command_data[1]);
			show_alert(
				"UPDATE IN PROGRESS",
				"Updating firmware: "+progress+"% done.<br><br>Do not disconnect your Emotiscope from power or the internet until the update is complete.",
				"PLEASE WAIT",
				function(){
					// nothing
				}
			);
		}
		else if(command_type == "ota_filesystem_progress"){
			let progress = parseInt(command_data[1]);
			if(progress == 100){
				window.location.reload();
			}
			show_alert(
				"UPDATE IN PROGRESS",
				"Updating filesystem: "+progress+"% done.<br><br>Do not disconnect your Emotiscope from power or the internet until the update is complete.",
				"PLEASE WAIT",
				function(){
					// nothing
				}
			);
		}
		else if(command_type == "version"){
			let version = command_data[1];
			document.getElementById("version_number").innerHTML = "Version: "+version;
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

function send_menu_toggle_change(toggle_name){
	let new_state = +(document.getElementById(toggle_name).checked);
	transmit(`set|${toggle_name}|${new_state}`);
}

function transmit(message){
	console.log(`TX: ${message}`);
	//document.getElementById("device_preview").innerHTML = message;
	ws.send(message);
}

// Function to transmit events
//function transmit(event_name) {
//    console.log(event_name); // Replace this with the actual implementation
//}

// Function to handle touch events on the header logo
function setup_header_logo_touch() {
    const header_logo = document.getElementById('header_logo');
    let touch_timer = null;
    let touch_active = false;

    header_logo.addEventListener('touchstart', function(e) {
        if (touch_active) return; // Ignore if another touch is already active
        touch_active = true;
        touch_timer = setTimeout(function() {
            transmit('button_hold');
			trigger_vibration(100);

			if(standby_mode == false){
				console.log("ENTERING STANDBY");
				standby_mode = true;
				document.getElementById("header_logo").style.color = "#5495d761";
			}
			else if(standby_mode == true){
				console.log("EXITING STANDBY");
				standby_mode = false;
				document.getElementById("header_logo").style.color = "var(--primary)";
			}

            touch_timer = null; // Clear the timer once the function is called
        }, 500); // Set timeout for 500ms
    });

    header_logo.addEventListener('touchend', function(e) {
        if (touch_timer) {
            clearTimeout(touch_timer); // Clear the timer if the touch ends before 500ms
            transmit('button_tap');

			var num_modes = modes.length;
			let next_mode = (configuration.current_mode + 1) % num_modes;
			configuration.current_mode = next_mode;
			let next_mode_name = modes[next_mode]; 
			document.getElementById("current_mode").innerHTML = next_mode_name;

        }
        touch_active = false; // Allow new touches
    });
}

// Function to handle touch events on the device icon
function setup_device_icon_touch() {
    const device_icon = document.getElementById('device_icon');
    let touch_timer = null;
    let touch_active = false;

    device_icon.addEventListener('touchstart', function(e) {
        if (touch_active) return; // Ignore if another touch is already active
        touch_active = true;
        touch_timer = setTimeout(function() {
            transmit('button_hold');
			trigger_vibration(100);

			if(standby_mode == false){
				console.log("ENTERING STANDBY");
				standby_mode = true;
				document.getElementById("header_logo").style.color = "#5495d761";
			}
			else if(standby_mode == true){
				console.log("EXITING STANDBY");
				standby_mode = false;
				document.getElementById("header_logo").style.color = "var(--primary)";
			}

            touch_timer = null; // Clear the timer once the function is called
        }, 500); // Set timeout for 500ms
    });

    device_icon.addEventListener('touchend', function(e) {
        if (touch_timer) {
            clearTimeout(touch_timer); // Clear the timer if the touch ends before 500ms
            transmit('button_tap');

			var num_modes = modes.length;
			let next_mode = (configuration.current_mode + 1) % num_modes;
			configuration.current_mode = next_mode;
			let next_mode_name = modes[next_mode]; 
			document.getElementById("current_mode").innerHTML = next_mode_name;

        }
        touch_active = false; // Allow new touches
    });
}

function reconnect_websockets(){
	if(reconnecting == false){
		reconnecting = true;
		if(AUTO_RECONNECT == true){
			try{
				ws.close();
			}
			catch(e){
				console.log("ERROR: "+e);
			}

			set_ui_locked_state(true);
			setTimeout(function(){
				window.location.reload();
			}, 2000);
		}
	}
}

function open_websockets_connection_to_device(){
	console.log("CONNECTING TO "+device_ip);
	ws = new WebSocket("ws://"+device_ip+":80/ws");
	document.getElementById("device_nickname").innerHTML = device_ip;

	ws.onopen = function(e) {
		console.log("[open] Connection established");
		connection_pending = false;

		transmit("get|version");
	};

	ws.onmessage = function(event) {
		if(event.data != "pong"){
			console.log(`RX: ${event.data}`);
		}
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
			reconnect_websockets();
		}
	};

	ws.onerror = function(error) {
		console.log(`[error]`);
		reconnect_websockets();
	};
}

// Register the touch event listeners on page load
document.addEventListener('DOMContentLoaded', function() {
    setup_header_logo_touch();
	setup_device_icon_touch();
});