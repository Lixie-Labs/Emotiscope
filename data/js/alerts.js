function show_alert(alert_title, alert_description, alert_button_text, alert_button_action){
	console.log("ACTION:");
	console.log(alert_button_action);
	let title = document.getElementById("alert_panel_title");
	let description = document.getElementById("alert_panel_description");
	let button = document.getElementById("alert_panel_button");
	let alert_dimmer = document.getElementById("alert_dimmer");

	title.innerHTML = alert_title;
	description.innerHTML = alert_description;
	button.innerHTML = alert_button_text;

	button.onclick = alert_button_action;
	console.log(button.onclick);

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