// Get the base URL of the current page
var base_url = window.location.origin;
var mac_str = "";

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

(function() {
    var first_load = true;

    // Register the touch event listeners on page load
    document.addEventListener('APP_LOADED', function() {	
		if(first_load == true){
			first_load = false;
			console.log("APP_LOADED fetch_mac.js");
			get_mac_string();
			setTimeout(get_mac_string, 10000);
		}
	});
})();