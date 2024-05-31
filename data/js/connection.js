var emotiscope_connected = false;
var ws = null;
var check_event_timeout_interval = null;
var device_ip = null;
var last_event_time = null;
var websockets_open = false;
var queued_packets = [];
var processing_command_queue = false;

var system_state = {
	"stats": {},
	"settings": {},
	"light_modes": {},
};

var setting_gallery = [];

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
							if(setting.value != item_value){
								setting.value = item_value;
								setting.draw();
							}
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

							setting.draw();
						}
						catch(e){
							console.log("ERROR SPAWNING SETTING: "+setting_name+" "+e);
						}
					}
				}

				// Update all settings in the gallery
				for( let j = 0; j < setting_gallery.length; j++ ){
					//setting_gallery[j].draw();
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

	let current_mode_index = system_state.settings.current_mode.value;
	let current_mode_name = Object.keys(system_state.light_modes)[current_mode_index];
	let current_mode_pretty_name = system_state.light_modes[current_mode_name].pretty_name;

	document.getElementById("current_mode").innerHTML = current_mode_pretty_name;

	console.log(system_state);
	//console.log(setting_gallery);
	//console.log("CPU_TEMP: " + system_state.stats.cpu_temp);

	//document.getElementById("device_nickname").innerHTML = device_ip;
	document.getElementById("device_nickname").innerHTML = "FPS CPU: "+parseInt(system_state.stats.fps_cpu)+" GPU: "+parseInt(system_state.stats.fps_gpu)+" TEMP: "+parseInt(system_state.stats.cpu_temp)+"C";
	//document.getElementById("device_nickname").innerHTML = "WS CLIENTS: "+parseInt(system_state.stats.ws_clients);
}

function parse_event(message){
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

		console.log("EMOTISCOPE CONNECTED");
		document.getElementById("device_nickname").innerHTML = device_ip;
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

function connect_to_emotiscope(){
	console.log("CONNECTING TO EMOTISCOPE");
	emotiscope_connected = false;
	device_ip = window.location.hostname;
	
	window.event_source = new EventSource("http://"+device_ip+"/events");

	// Handle the open event
	window.event_source.onopen = function(event) {
		console.log("[EventSource] Connection opened");
		set_ui_locked_state(false);
		check_event_timeout_interval = setInterval(check_event_timeout, 1000);
		emotiscope_connected = true;
	};

	// Handle the message event
	window.event_source.onmessage = function(event) {
		last_event_time = Date.now();
		parse_event(event.data);
	};

	// Handle the error event
	window.event_source.onerror = function(event) {
		console.error("[EventSource] Error occurred:", event);
		if (event.readyState == EventSource.CLOSED) {
			console.log("[EventSource] Connection closed");
		}

		window.event_source.close();
	};
}

function check_event_timeout(){
	if(Date.now() - last_event_time >= 1000){
		clearInterval(check_event_timeout_interval);
		window.event_source.close();
		console.log("EVENT TIMEOUT");

		set_ui_locked_state(true);
		connect_to_emotiscope();
	}
}

function send_queued_packets(){
	for( let i = 0; i < queued_packets.length; i++ ){
		if(queued_packets[i].sent == false){
			console.log("TX: " + queued_packets[i].message);
			try{
				ws.send(queued_packets[i].message);
				queued_packets[i].sent = true;
			}
			catch(e){
				console.log("ERROR SENDING QUEUED WEBSOCKETS: "+e);
			}
		}
	}

	// Remove all sent packets
	queued_packets = queued_packets.filter(packet => packet.sent == false);
}

function wstx(message){
	queued_packets.push(
		{
			"message":message,
			"sent":false
		}
	);
	console.log("TX QUEUED: " + message);

	process_command_queue();
}

function open_websockets_connection(){
	if(ws == null || ws.readyState == 3){
		console.log("OPENING WEBSOCKETS");
		websockets_open = true;

		ws = new WebSocket("ws://"+device_ip+":80/ws");
		ws.onopen = function(){
			console.log("WEBSOCKETS OPEN");
			send_queued_packets();
			
		};
		ws.onmessage = function(event){
			console.log("RX: " + event.data);
			parse_event(event.data);
		};
		ws.onclose = function(){
			console.log("WEBSOCKETS CLOSED");
			websockets_open = false;
		};
		ws.onerror = function(){
			console.log("WEBSOCKETS ERROR");
			websockets_open = false;
		}
	}
	else{
		console.log("WEBSOCKETS ALREADY OPEN");
		send_queued_packets();
	}
}

function close_websockets_connection(){
	websockets_open = false;
	try{		
		ws.close();
	}
	catch(e){
		console.log("ERROR CLOSING WEBSOCKETS: "+e);
	}
}

function process_command_queue(){
	console.log("queued packets: "+queued_packets.length);
	let queue_length = queued_packets.length;
	if(queue_length > 0){
		open_websockets_connection();
	}
}

(function() {
    var first_load = true;

    // Register the touch event listeners on page load
    document.addEventListener('APP_LOADED', function() {	
        if(first_load == true){
            first_load = false;
            console.log("APP_LOADED connection.js");

			connect_to_emotiscope();

			// Add event listeners for mousedown and touchstart events
			document.addEventListener('mousedown', function(){
				wstx("IND"); // Indicate touch
			});
			document.addEventListener('touchstart', function(){
				wstx("IND"); // Indicate touch
			});
        }
    });
})();
