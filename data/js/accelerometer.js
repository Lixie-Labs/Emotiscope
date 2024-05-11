// Initialize the on_surface variable
let on_surface = false;

// Function to check if the phone is face down
function check_device_orientation(event) {
	// Read the accelerometer values
	let x = event.beta;  // Beta value is the front-to-back tilt in degrees, where front is positive
	let y = event.gamma; // Gamma value is the left-to-right tilt in degrees, where right is positive

	// Check if the device is lying perfectly level on a table
	if (Math.abs(x) <= 10 && Math.abs(y) <= 10) {
		if (!on_surface) { // Update only if state changes
			on_surface = true;
			document.getElementById('accelerometer').textContent = 'On a flat surface';
		}
	} else {
		if (on_surface) { // Update only if state changes
			on_surface = false;
			document.getElementById('accelerometer').textContent = 'Not on a flat surface';
		}
	}
}

(function() {
    var first_load = true;

    // Register the touch event listeners on page load
    document.addEventListener('APP_LOADED', function() {
		if(first_load == true){
            first_load = false;
            console.log("APP_LOADED accelerometer.js");
			
			// Add an event listener for device orientation changes
			if (window.DeviceOrientationEvent) {
				document.getElementById("accelerometer").innerHTML = "Device Orientation API supported";
				window.addEventListener("deviceorientation", check_device_orientation, true);
			} else {
				document.getElementById("accelerometer").innerHTML = "Device Orientation API not supported";
			}
		}
	});
})();