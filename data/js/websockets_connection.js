var emotiscope_connected = false;
var ws = null;
var attempt_to_reach_emotiscope_interval = null;
var device_ip = null;

// Function to handle touch events on the device icon
function setup_top_touch_listener(div_id) {
    const device_icon = document.getElementById(div_id);
    let touch_timer = null;
    let touch_active = false;

    device_icon.addEventListener('touchstart', function(e) {
        if (touch_active) return; // Ignore if another touch is already active
        touch_active = true;
        touch_timer = setTimeout(function() {
            wstx('button_hold');
			trigger_vibration(100);

			if(standby_mode == false){
				//console.log("ENTERING STANDBY");
				standby_mode = true;
				document.getElementById("header_logo").style.color = "#5495d761";
			}
			else if(standby_mode == true){
				//console.log("EXITING STANDBY");
				standby_mode = false;
				document.getElementById("header_logo").style.color = "var(--primary)";
			}

            touch_timer = null; // Clear the timer once the function is called
        }, 500); // Set timeout for 500ms
    });

    device_icon.addEventListener('touchend', function(e) {
        if (touch_timer) {
            clearTimeout(touch_timer); // Clear the timer if the touch ends before 500ms
            wstx('button_tap');

			if(standby_mode == true){
				standby_mode = false;
				document.getElementById("header_logo").style.color = "var(--primary)";
			}
        }
        touch_active = false; // Allow new touches
    });
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

function set_mode(mode_name){
	wstx(`set|mode|${mode_name}`);
}

function increment_mode(){
	wstx(`increment_mode`);
}

function wstx(message){
	ws.send(message);
	console.log("TX: " + message);
	//add_to_log("TX: " + message);
}

function wsrx(message){
	console.log("RX: " + message);
	//add_to_log("RX: " + message);

	var message_items = message.split("|");
	var message_type = message_items[0];

	if( message_type == "emotiscope" ){
		emotiscope_connected = true;
		clearInterval(attempt_to_reach_emotiscope_interval);

		console.log("EMOTISCOPE CONNECTED");
		document.getElementById("device_nickname").innerHTML = device_ip;

		wstx("get|version");
		wstx("get|config");
	}
	else if(message_type == "clear_config"){
		// Clear client-side config JSON
		configuration = {};

		set_ui_locked_state(true);
	}
	else if(message_type == "new_config"){
		// Append new config key to client-side config JSON
		let config_key_name  = message_items[1];
		let config_data_type = message_items[2];
		let config_value_raw = message_items[3];
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
	else if(message_type == "config_ready"){
		wstx("get|modes");
	}
	else if(message_type == "clear_modes"){
		modes = [];
	}
	else if(message_type == "new_mode"){
		let mode_index = parseInt(message_items[1]);
		let mode_type  = parseInt(message_items[2]);
		let mode_name  = message_items[3];

		modes.push({
			"mode_index":mode_index,
			"mode_type":mode_type,
			"mode_name":mode_name
		});
	}
	else if(message_type == "modes_ready"){
		wstx("get|sliders");
	}
	else if(message_type == "clear_sliders"){
		// Force close UI if it's open
		wstx(`touch_end`);
		wstx(`slider_touch_end`);
		
		sliders = [];
	}
	else if(message_type == "new_slider"){
		let slider_name = message_items[1];
		let slider_min  = parseFloat(message_items[2]);
		let slider_max  = parseFloat(message_items[3]);
		let slider_step = parseFloat(message_items[4]);

		sliders.push({
			"name":slider_name,
			"min":slider_min,
			"max":slider_max,
			"step":slider_step
		});
	}
	else if(message_type == "sliders_ready"){
		wstx("get|toggles");
	}
	else if(message_type == "clear_toggles"){
		toggles = [];
	}
	else if(message_type == "new_toggle"){
		let toggle_name = message_items[1];

		toggles.push({
			"name":toggle_name
		});
	}
	else if(message_type == "toggles_ready"){
		wstx("get|menu_toggles");
	}
	else if(message_type == "clear_menu_toggles"){
		menu_toggles = [];
	}
	else if(message_type == "new_menu_toggle"){
		let toggle_name = message_items[1];

		menu_toggles.push({
			"name":toggle_name
		});
	}
	else if(message_type == "menu_toggles_ready"){
		//console.log("DATA SYNC COMPLETE!");
		//ping_server();
		//setInterval(check_pong_timeout, 100);
		
		render_controls();
		set_ui_locked_state(false);
	}
	else if(message_type == "reload_config"){
		wstx("get|config");
	}
	else if(message_type == "fps_cpu"){
		let FPS = message_items[1];
		document.getElementById("CPU_FPS").innerHTML = `CPU FPS: ${FPS}`;
	}
	else if(message_type == "fps_gpu"){
		let FPS = message_items[1];
		document.getElementById("GPU_FPS").innerHTML = `GPU FPS: ${FPS}`;
	}
	else if(message_type == "heap"){
		let heap = message_items[1];
		document.getElementById("HEAP").innerHTML = `HEAP: ${heap}`;
	}
	else if(message_type == "update_available"){
		pongs_halted = true;
		show_alert(
			"UPDATE AVAILABLE",
			"An update is available for your Emotiscope!<br><br>Click below to download the latest firmware.",
			"UPDATE NOW",
			function(){
				hide_alert();
				wstx("perform_update");
			}
		);
	}
	else if(message_type == "no_updates"){
		show_alert(
			"ALREADY UP-TO-DATE",
			"Your Emotiscope is already running the latest firmware!<br><br>(But follow @lixielabs on social media to keep <em>yourself</em> up-to-date on new features and improvements that are coming!)",
			"DAMN, OK",
			function(){
				hide_alert();
			}
		);
	}
	else if(message_type == "ota_firmware_progress"){
		let progress = parseInt(message_items[1]);
		show_alert(
			"UPDATE IN PROGRESS",
			"Updating firmware: "+progress+"% done.<br><br>Do not disconnect your Emotiscope from power or the internet until the update is complete.",
			"PLEASE WAIT",
			function(){
				// nothing
			}
		);
	}
	else if(message_type == "ota_filesystem_progress"){
		let progress = parseInt(message_items[1]);
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
	else if(message_type == "version"){
		let version = message_items[1];
		document.getElementById("version_number").innerHTML = "Version: "+version;
	}

	else{
		console.log("UNRECOGNIZED MESSAGE TYPE: " + message_type);
	}
}

function attempt_to_reach_emotiscope() {
	console.log("attempt_to_reach_emotiscope()");
	if(emotiscope_connected == false){
		// Send a message to the server
		wstx("emotiscope?");
	}
}

function connect_to_emotiscope() {
	var websockets_server = "ws://"+device_ip+":80/ws";
	console.log("connecting to emotiscope at " + websockets_server);

	ws = new WebSocket(websockets_server);

	ws.onopen = function() {
		console.log("Websockets connection established");
		emotiscope_connected = false;

		attempt_to_reach_emotiscope();
		attempt_to_reach_emotiscope_interval = setInterval(attempt_to_reach_emotiscope, 250);

		setInterval(function(){
			wstx("EMO~set_config|Softness|0.250");
		}, 5000);
	};

	ws.onmessage = function(event) {
		wsrx(event.data);
	};

	ws.onclose = function() {
		console.log("Websockets connection closed");
	};

	ws.onerror = function(event) {
		console.log("Websockets error: " + event);
	};
}

(function() {
    var first_load = true;

    // Register the touch event listeners on page load
    document.addEventListener('APP_LOADED', function() {	
        if(first_load == true){
            first_load = false;
            console.log("APP_LOADED websockets_connection.js");

			device_ip = window.location.hostname;

			setup_top_touch_listener("header_logo");
			setup_top_touch_listener("device_icon");

			connect_to_emotiscope();
        }
    });
})();