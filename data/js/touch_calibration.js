function touch_calibration_step(step){
	if(step == 0){
		show_alert(
			"TOUCH CALIBRATION",
			"Please remove your hands from Emotiscope and press OK to continue.",
			"OK",
			function(){touch_calibration_step(1)}
		);
	}
	else if(step == 1){
		// Do ambient touch calibration here
		transmit('start_touch_calibration');
		transmit('get|touch_vals');
		setTimeout(function(){
			touch_low[0] = touch_vals[0];
			touch_low[1] = touch_vals[1];
			touch_low[2] = touch_vals[2];

			show_alert(
				"TOP CALIBRATION",
				"Press MEASURE while holding a finger on the top of Emotiscope...",
				"MEASURE",
				function(){touch_calibration_step(2)}
			);
		
		}, 250);
	}
	else if(step == 2){
		// Do top touch calibration here
		transmit('get|touch_vals');
				
		setTimeout(function(){
			touch_high[1] = touch_vals[1];

			show_alert(
				"LEFT CALIBRATION",
				"Press MEASURE while holding a finger on the left side of Emotiscope...",
				"MEASURE",
				function(){touch_calibration_step(3)}
			);
		}, 250);
	}
	else if(step == 3){
		// Do left touch calibration here
		transmit('get|touch_vals');
				
		setTimeout(function(){
			touch_high[0] = touch_vals[0];
		
			show_alert(
				"RIGHT CALIBRATION",
				"Press MEASURE while holding a finger on the right side of Emotiscope...",
				"MEASURE",
				function(){touch_calibration_step(4)}
			);
		}, 250);
	}
	else if(step == 4){
		// Do right touch calibration here
		transmit('get|touch_vals');

		setTimeout(function(){
			touch_high[2] = touch_vals[2];

			show_alert(
				"CALIBRATION DONE",
				"All set! Press OK to continue.",
				"OK",
				hide_alert
			);

			// Done
			//console.log(touch_low);
			//console.log(touch_high);

			let ambient_threshold_left   = Math.round(touch_low[0]*0.9 + touch_high[0]*0.1);
			let ambient_threshold_center = Math.round(touch_low[1]*0.9 + touch_high[1]*0.1);
			let ambient_threshold_right  = Math.round(touch_low[2]*0.9 + touch_high[2]*0.1);

			let touch_threshold_left   = Math.round(touch_low[0]*0.25 + touch_high[0]*0.75);
			let touch_threshold_center = Math.round(touch_low[1]*0.25 + touch_high[1]*0.75);
			let touch_threshold_right  = Math.round(touch_low[2]*0.25 + touch_high[2]*0.75);
			
			transmit('set|touch_thresholds|'+ambient_threshold_left+'|'+ambient_threshold_center+'|'+ambient_threshold_right+'|'+touch_threshold_left+'|'+touch_threshold_center+'|'+touch_threshold_right);
			transmit('end_touch_calibration');
		}, 250);
	}
}