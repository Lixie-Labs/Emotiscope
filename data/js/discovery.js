const max_retry_attempts = 3;
let retry_count = 0;
const base_delay = 250;

(function() {
    var first_load = true;

    // Register the touch event listeners on page load
    document.addEventListener('APP_LOADED', function() {
		if(first_load == true){
            first_load = false;
			console.log("APP_LOADED discovery.js");
		}
	});
})();