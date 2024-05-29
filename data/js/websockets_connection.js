var emotiscope_connected = false;
var ws = null;
var attempt_to_reach_emotiscope_interval = null;
var device_ip = null;

var system_state = {
	"stats": {},
	"settings": {},
	"light_modes": {},
};

var setting_gallery = [];

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

function parse_emotiscope_packet(packet){
	let chunks = packet.split("~");
	for( let i = 0; i < chunks.length; i++ ){
		let chunk = chunks[i];
		
		if(chunk != "EMO"){
			// First item is the chunk name
			let chunk_name = chunk.split("|")[0];
			// All other items are sections
			let sections = chunk.split("|").slice(1);

			if(chunk_name == "stats"){
				let num_sections = sections.length;
				let stat_length = 2;
				let num_stats = num_sections / stat_length;

				for( let j = 0; j < num_stats; j++ ){
					let stat_name  = sections[j*stat_length+0];
					let stat_value = sections[j*stat_length+1];

					system_state.stats[stat_name] = stat_value;
				}
			}
			else if(chunk_name == "config"){
				let num_sections = sections.length;
				let config_length = 4;
				let num_config_items = num_sections / config_length;

				for( let j = 0; j < num_config_items; j++ ){
					let item_name    = sections[j*config_length+0];
					let item_type    = sections[j*config_length+1];
					let item_ui_type = sections[j*config_length+2];
					let item_value   = sections[j*config_length+3];

					if(item_type == "i32"){
						item_value = parseInt(item_value);
					}
					else if(item_type == "u32"){
						item_value = parseInt(item_value);
					}
					else if(item_type == "f32"){
						item_value = parseFloat(item_value);
					}
					
					let item_name_clean = item_name.replace(/\s+/g, '_').toLowerCase();
					
					// if key already exists, update the value
					if(system_state.settings[item_name_clean]){
						system_state.settings[item_name_clean].value = item_value;

						// UPDATE SETTING HERE
						const setting = setting_gallery.find(setting => setting.name === item_name_clean);
						if(setting){
							setting.value = item_value;
						}
					}

					// otherwise, create a new key
					else{
						system_state.settings[item_name_clean] = {
							"name": item_name_clean,
							"pretty_name": item_name,
							"type": item_type,
							"ui_type": item_ui_type,
							"value": item_value,
						};
					}
				}

				var setting_order = [
					// Sliders
					"Brightness",
					"Softness",
					"Color",
					"Color Range",
					"Saturation",
					"Warmth",
					"Background",
					"Blur",
					"Speed",

					// Toggles
					"Mirror Mode",
					"Reverse Color Range",
					"Auto Color Cycle",
				]

				// Spawn all settings not already in the gallery, in the correct order
				for( let j = 0; j < setting_order.length; j++ ){
					let setting_pretty_name = setting_order[j];

					// if setting not already spawned
					if(!setting_gallery.find(setting => setting.pretty_name === setting_pretty_name)){
						try{
							let setting_name = setting_pretty_name.replace(/\s+/g, '_').toLowerCase();
							let setting_type = system_state.settings[setting_name].type;
							let setting_ui_type = system_state.settings[setting_name].ui_type;
							let setting_value = system_state.settings[setting_name].value;

							const setting = new Setting(setting_name, setting_pretty_name, setting_type, setting_ui_type, setting_value);
							setting_gallery.push(setting);
						}
						catch(e){
							console.log("ERROR SPAWNING SETTING: "+setting_name+" "+e);
						}
					}
				}

				// Update all settings in the gallery
				for( let j = 0; j < setting_gallery.length; j++ ){
					setting_gallery[j].draw();
				}

				set_ui_locked_state(false);
			}
			else if(chunk_name == "modes"){
				let num_sections = sections.length;
				let mode_length = 2;
				let num_modes = num_sections / mode_length;

				for( let j = 0; j < num_modes; j++ ){
					let mode_name = sections[j*mode_length+0];
					let mode_type = sections[j*mode_length+1];

					let mode_name_clean = mode_name.replace(/\s+/g, '_').toLowerCase();

					system_state.light_modes[mode_name_clean] = {
						"name": mode_name_clean,
						"pretty_name": mode_name,
						"type": mode_type,
					};
				}
			}
		}
	}

	console.log(system_state);
	console.log(setting_gallery);
	console.log("CPU_TEMP: " + system_state.stats.cpu_temp);
}

function wstx(message){
	ws.send(message);
	//console.log("TX: " + message);
	//add_to_log("TX: " + message);
}

function wsrx(message){
	//console.log("RX: " + message);
	//add_to_log("RX: " + message);

	var message_items = message.split("|");
	var message_type = message_items[0];

	// check if first three chars match "EMO"
	if(message_type.substring(0, 3) == "EMO"){
		parse_emotiscope_packet(message);
	}

	// other commands
	else if( message_type == "emotiscope" ){
		emotiscope_connected = true;
		clearInterval(attempt_to_reach_emotiscope_interval);

		console.log("EMOTISCOPE CONNECTED");
		document.getElementById("device_nickname").innerHTML = device_ip;

		wstx("get|version");
		wstx("get|config");
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