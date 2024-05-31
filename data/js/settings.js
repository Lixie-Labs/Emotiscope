class Setting {
    // Constructor method to initialize a Setting
    constructor(name, pretty_name, type, ui_type, initial_value) {
        this.name = name;
        this.pretty_name = pretty_name;
		this.type = type;
		this.ui_type = ui_type;
		this.value = initial_value;
    }

    // Method to return the Setting's value
    draw() {
		// If element doesn't exist, create it
		if (document.getElementById("setting_"+this.name) == null) {
			console.log("Creating setting: " + this.name);
			var setting = document.createElement("div");
			setting.id = "setting_"+this.name;
			setting.className = "setting";
			document.getElementById("setting_container").appendChild(setting);

			setting.addEventListener('touchstart', (e) => {
				//console.log('Touch start detected on '+this.name+' div.');
				wstx("IND");
			});
		
			// Touch end event
			setting.addEventListener('touchend', (e) => {
				//console.log('Touch end detected on '+this.name+' div.');
				wstx("IND");
			});
		
			// Touch move event
			setting.addEventListener('touchmove', (e) => {
				//console.log('Touch move detected on '+this.name+' div.');
				wstx("IND");
			});

			// Touch cancel event
			setting.addEventListener('touchcancel', (e) => {
				//console.log('Touch cancel detected on '+this.name+' div.');
				wstx("IND");
			});
		
			// Optional: Prevent default touch actions (like scrolling) if needed
			setting.addEventListener('touchstart', (e) => {
				e.preventDefault();
			});

			var setting_html;

			// Generate HTML for setting
			if(this.ui_type == "s") {
				setting_html = "<div class='slider' id='"+this.name+"'>" + this.pretty_name + ": " + this.value + "</div>";
				document.getElementById("setting_"+this.name).innerHTML = setting_html;
			}
			else if(this.ui_type == "t") {
				setting_html = "<div class='toggle' id='"+this.name+"'>" + this.pretty_name + ": " + this.value + "</div>";
				document.getElementById("setting_"+this.name).innerHTML = setting_html;
			}
			else{
				console.log("Invalid UI type for setting: " + this.name, this.ui_type);
			}
		}

		// If already exists:
		else {
			document.getElementById(this.name).innerHTML = this.pretty_name + ": " + this.value;

			if(this.ui_type == "s") {
				// Draw vertical slider using CSS gradient with sharp transition at value point
				document.getElementById(this.name).style.background = "linear-gradient(to bottom, #000000 0%, #000000 " + ((1 - this.value)*100) + "%, #FFFFFF " + ((1 - this.value)*100) + "%, #FFFFFF 100%)";
			}
		}
		
		return;
	}

    // Static method - it belongs to the class, not the instance
    static begin() {
		// DOES NOTHING
		return;
    }
}