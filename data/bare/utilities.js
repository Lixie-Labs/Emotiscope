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