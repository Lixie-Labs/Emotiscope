// ############################################################################
// ############################################################################
//                                  __  _
//            ___  ____ ___  ____  / /_(_)_____________  ____  ___
//           / _ \/ __ `__ \/ __ \/ __/ / ___/ ___/ __ \/ __ \/ _ \
//          /  __/ / / / / / /_/ / /_/ (__  ) /__/ /_/ / /_/ /  __/
//          \___/_/ /_/ /_/\____/\__/_/____/\___/\____/ .___/\___/
//              Audio-visual engine by @lixielabs    /_/
//              Released under the GPLv3 Licence
//
// ############################################################################
// ## VERSION #################################################################

#define VERSION_MAJOR ( 1 )
#define VERSION_MINOR ( 0 )
#define VERSION_PATCH ( 0 )

// ############################################################################
// ## DEPENDENCIES ############################################################

// External dependencies
//#include <FastLED.h> // ......... You've served me well, but you're not compatible with the 3.0.0-alpha ESP32 board def yet, and I need the IDF 5.1.2 for this madness to even work. I cobbled my own RMT LED driver for now.
#include <PsychicHttp.h> // ....... Handling the web-app HTTP and WS
#include <HTTPClient.h> // ........ Used to make POST requests to the discovery server
#include <ESPmDNS.h> // ........... Used for "emotiscope.local" domain name
#include <Ticker.h> // ............ For timing functions
#include <DNSServer.h> // ......... Captive portal functionality
#include <WiFi.h> // .............. WiFi connection library
#include <esp_dsp.h> // ........... Fast SIMD-style array math
#include <esp_wifi.h> // .......... WiFi, but like - the hardware side of it

// Internal dependencies
#include "global_defines.h" // .... Compile-time configuration
#include "types.h" // ............. typedefs for things like CRGBFs
#include "profiler.h" // .......... Developer tools, measures the execution of functions
#include "sliders.h" // ........... Handles sliders that appear in the web app
#include "toggles.h" // ........... Handles toggles that appear in the web app
#include "menu_toggles.h" // ...... Toggles that appear in the main menu
#include "filesystem.h" // ........ LittleFS functions
#include "configuration.h" // ..... Storing and retreiving your settings
#include "utilities.h" // ......... Custom generic math functions
#include "system.h" // ............ Lowest-level firmware functions
#include "touch.h" // ............. Handles capacitive touch input
#include "indicator.h" // ......... Little light bulb
#include "led_driver.h" // ........ Low-level LED communication, (ab)uses RMT for non-blocking output
#include "leds.h" // .............. LED dithering, effects, filters
#include "ui.h" // ................ Draws UI elements to the LEDs like indicator needles
#include "microphone.h" // ........ For gathering audio chunks from the microphone
#include "vu.h" // ................ Tracks music loudness from moment to moment
#include "goertzel.h" // .......... GDFT or God Damn Fast Transform is implemented here
#include "tempo.h" // ............. Comupation of (and syncronization) to the music tempo
#include "audio_debug.h" // ....... Print audio data over UART
#include "screensaver.h" // ....... Colorful dots play on screen when no audio is present
#include "standby.h" // ........... Handles sleep/wake + animations
#include "lightshow_modes.h" // ... Definition and handling of lightshow modes
#include "commands.h" // .......... Queuing and parsing of commands recieved
#include "wireless.h" // .......... Communication with your network and the web-app

// Loops
#include "cpu_core.h" // Audio
#include "gpu_core.h" // Video
#include "web_core.h" // Wireless

// Development Notes
#include "notes.h"

// ############################################################################
// ## CODE ####################################################################

// One core to run audio and web server ---------------------------------------
void loop() {
	run_cpu(); // (cpu_core.h)
	run_web(); // (web_core.h)	
}

// One core to run graphics ---------------------------------------------------
void loop_gpu(void *param) {
	for (;;) {
		run_gpu(); // (gpu_core.h)
	}
}

// EVERYTHING BEGINS HERE ON BOOT ---------------------------------------------
void setup() {
	// (system.h) Initialize all peripherals
	init_system();

	// Start the second core as a dedicated webserver
	(void)xTaskCreatePinnedToCore(loop_gpu, "loop_gpu", 8192, NULL, 0, NULL, 0);
}