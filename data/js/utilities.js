function show_page(page_div_id, page_title){
	let page_div = document.getElementById(page_div_id);
	page_div.style.opacity = 1.0;
	page_div.style.pointerEvents = "all";
	document.getElementById("header_logo").innerHTML = page_title;
}

function hide_page(page_div_id){
	let page_div = document.getElementById(page_div_id);
	page_div.style.opacity = 0.0;
	page_div.style.pointerEvents = "none";
	document.getElementById("header_logo").innerHTML = "emotiscope";
}

function show_alert(alert_title, alert_description, alert_button_text, alert_button_action){
	//console.log("ACTION:");
	//console.log(alert_button_action);
	let title = document.getElementById("alert_panel_title");
	let description = document.getElementById("alert_panel_description");
	let button = document.getElementById("alert_panel_button");
	let alert_dimmer = document.getElementById("alert_dimmer");

	title.innerHTML = alert_title;
	description.innerHTML = alert_description;
	button.innerHTML = alert_button_text;

	button.onclick = alert_button_action;
	//console.log(button.onclick);

	let panel_div = document.getElementById("alert_panel");
	panel_div.style.opacity = 1.0;
	panel_div.style.pointerEvents = "all";

	alert_dimmer.style.opacity = 1.0;
	alert_dimmer.style.pointerEvents = "all";
}

function hide_alert(){
	let panel_div = document.getElementById("alert_panel");
	panel_div.style.opacity = 0.0;
	panel_div.style.pointerEvents = "none";

	alert_dimmer.style.opacity = 0.0;
	alert_dimmer.style.pointerEvents = "none";
}

// Function to pretty print a JSON object into a specified div
function pretty_print_json(label, json_object, target_div_id) {
    // Convert JSON object to a pretty-printed string with 2-space indentation
    var pretty_json = JSON.stringify(json_object, null, 2);

    // Replace spaces and line breaks for HTML display
    pretty_json = pretty_json.replace(/ /g, '&nbsp;').replace(/\n/g, '<br>');

    // Set the pretty-printed JSON as the content of the target div
    document.getElementById(target_div_id).innerHTML = `<strong>${label}</strong><br>`;
	document.getElementById(target_div_id).innerHTML += pretty_json;
}

// Function to truncate a float to a specified number of decimal places
function truncate_float(value, decimal_places) {
    // Check if input is a number and decimal_places is a non-negative integer
    if (typeof value !== 'number' || isNaN(value) || !Number.isInteger(decimal_places) || decimal_places < 0) {
        console.error('Invalid input');
        return NaN; // Return NaN for invalid input
    }

    var scale = Math.pow(10, decimal_places); // Calculate scaling factor
    return Math.trunc(value * scale) / scale; // Truncate and scale back
}

