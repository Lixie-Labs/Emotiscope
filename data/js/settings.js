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
			var setting = document.createElement("div");
			setting.id = "setting_"+this.name;
			document.getElementById("setting_container").appendChild(setting);
		}

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
		
		return;
	}

    // Static method - it belongs to the class, not the instance
    static begin() {
		// DOES NOTHING
		return;
    }
}