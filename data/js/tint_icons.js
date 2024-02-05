function tint_svg_images() {
    // Select all <img> elements
    const images = document.querySelectorAll('img');

    images.forEach(img => {
        const src = img.getAttribute('src');
        const class_names = img.className.split(' ');
        const img_id = img.getAttribute('id');

		// Get the computed styles of the <img> to preserve dimensions
        const img_height = getComputedStyle(img).height;
        const img_width = getComputedStyle(img).width;

        // Check if image source is SVG and has a class starting with 'tint_'
        if (src.endsWith('.svg') && class_names.some(class_name => class_name.startsWith('tint_'))) {
            class_names.forEach(class_name => {
                if (class_name.startsWith('tint_')) {
                    const color_key = class_name.split('_')[1]; // Get the suffix after 'tint_'
                    const color_value = getComputedStyle(document.documentElement).getPropertyValue(`--${color_key}`).trim(); // Get the CSS variable value

                    // Load the SVG content and set the stroke color and dimensions
                    fetch(src)
                        .then(response => response.text())
                        .then(svg_content => {
                            const svg_element = new DOMParser().parseFromString(svg_content, "image/svg+xml").documentElement;
                            svg_element.querySelectorAll('path').forEach(path => {
                                path.setAttribute('stroke', color_value); // Set stroke color
                            });

                            // Set SVG dimensions to match original <img>
                            svg_element.setAttribute('width', img_width);
                            svg_element.setAttribute('height', img_height);

                            // Create a container for the SVG, preserve classes and id
                            const svg_container = document.createElement('div');
                            svg_container.appendChild(svg_element);
                            svg_container.className = img.className; // Preserve all class names
                            if (img_id) svg_container.id = img_id; // Preserve the id if exists

                            img.replaceWith(svg_container); // Replace <img> with the new container
                        })
                        .catch(error => console.error('Error loading SVG:', error));
                }
            });
        }
    });
}

tint_svg_images();