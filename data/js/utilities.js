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

