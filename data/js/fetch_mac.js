// Get the base URL of the current page
const base_url = window.location.origin;
let mac_str = "";

function get_mac_string(){
	// Fetch the text result from the /mac endpoint
	fetch(`${base_url}/mac`)
		.then(response => {
			if (!response.ok) {
				throw new Error('Network response was not 200');
			}
			return response.text();
		})
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

window.addEventListener('DOMContentLoaded', (event) => {
	setTimeout(get_mac_string, 100);
});