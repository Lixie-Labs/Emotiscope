// Function to check if the device is in landscape mode
function is_landscape() {
    return window.innerWidth > window.innerHeight;
}

// Function to toggle the dimmer and emoticon
function toggle_dimmer(show) {
    const dimmer = document.getElementById('landscape-dimmer');

    if (show) {
        dimmer.style.opacity = '1.0';
        dimmer.style.visibility = 'visible';
    } else {
        dimmer.style.opacity = '0';
        setTimeout(() => { dimmer.style.visibility = 'hidden'; }, 500); // Ensure the dimmer is only hidden after the fade-out transition
    }
}

// Function to initialize the dimmer and event listeners
function setup_landscape_listener() {
    // Create the dimmer div and add it to the body
    const dimmer = document.createElement('div');
    dimmer.id = 'landscape-dimmer';
    dimmer.style.position = 'fixed';
    dimmer.style.top = '0';
    dimmer.style.left = '0';
    dimmer.style.width = '100%';
    dimmer.style.height = '100%';
    dimmer.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
    dimmer.style.display = 'flex';
    dimmer.style.alignItems = 'center';
    dimmer.style.justifyContent = 'center';
    dimmer.style.fontSize = '5em';
    dimmer.style.transition = 'opacity 500ms';
    dimmer.style.visibility = 'hidden';
    dimmer.style.opacity = '0';
	dimmer.style.color = 'var(--accent)';
    dimmer.innerHTML = ':(';
    document.body.appendChild(dimmer);

    // Add event listener for window resize
    window.addEventListener('resize', () => {
        toggle_dimmer(is_landscape());
    });

    // Initial check in case the page loads in landscape mode
    toggle_dimmer(is_landscape());
}

// Initialize the listener on page load
window.addEventListener('DOMContentLoaded', setup_landscape_listener);
