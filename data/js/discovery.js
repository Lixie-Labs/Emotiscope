const max_retry_attempts = 3;
let retry_count = 0;
const base_delay = 250;

function fetch_and_redirect() {
	console.log("fetch");
	const target_url = 'https://app.emotiscope.rocks/discovery/';

	fetch(target_url, { redirect: 'follow' })
		.then(response => {
			if (!response.ok) {
				print("BAD RESPONSE");
				output("Network response was not ok");

				device_ip = window.location.hostname;
				connection_start_time = performance.now();
				connection_pending = true;
				setInterval(check_connection_timeout, 100);

				open_websockets_connection_to_device();
				
				throw new Error('Network response was not ok');
			}
			return response.json();
		})
		.then(data => {
			if (data.length > 0) {
				console.log(data);
				retry_count = 0;
				console.log("Device seen on network @ "+data[0].local_ip+"!");

				device_ip = data[0].local_ip;
				connection_start_time = performance.now();
				connection_pending = true;
				setInterval(check_connection_timeout, 100);

				open_websockets_connection_to_device();
			} else {
				if (retry_count < max_retry_attempts) {
					let delay = base_delay * Math.pow(2, retry_count);
					console.log(`No devices seen on network, re-checking in ${delay}ms...`);
					setTimeout(fetch_and_redirect, delay);
					retry_count++;
				} else {
					console.log('Failed to fetch local IP after ' + max_retry_attempts + '!');
					show_connection_error();

					device_ip = window.location.hostname;
					connection_start_time = performance.now();
					connection_pending = true;
					setInterval(check_connection_timeout, 100);

					open_websockets_connection_to_device();
				}
			}
		})
		.catch(error => {
			if (retry_count < max_retry_attempts) {
				let delay = base_delay * Math.pow(2, retry_count);
				setTimeout(fetch_and_redirect, delay);
				retry_count++;
			} else {
				output('Failed to reach discovery server after ' + max_retry_attempts + ' attempts: ' + error.message);

				device_ip = window.location.hostname;
				connection_start_time = performance.now();
				connection_pending = true;
				setInterval(check_connection_timeout, 100);

				open_websockets_connection_to_device();
			}
		}
	);
}

// If page is fully loaded, fetch and redirect
document.addEventListener('DOMContentLoaded', (event) => {
	//fetch_and_redirect();
	device_ip = window.location.hostname;
	connection_start_time = performance.now();
	connection_pending = true;
	setInterval(check_connection_timeout, 100);

	open_websockets_connection_to_device();
});