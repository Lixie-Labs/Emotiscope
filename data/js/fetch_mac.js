// Get the base URL of the current page
const base_url = window.location.origin;

// Construct the URL for the /mac endpoint
const mac_url = `${base_url}/mac`;

let mac_str = "";

function get_mac_string(){
	// Fetch the text result from the /mac endpoint
	fetch(mac_url)
		.then(response => response.text())
		.then(result => {
			// Print the result to the console
			console.log("MAC: "+result);
			mac_str = result;

			document.getElementById("mac_address").innerHTML = "MAC: "+mac_str;
		})
		.catch(error => {
			// Handle any errors that occur during the fetch
			console.error('Error:', error);
		});
}

get_mac_string();