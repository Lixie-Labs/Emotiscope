// ############################################################################
// ############################################################################
//                                  __  _
//            ___  ____ ___  ____  / /_(_)_____________  ____  ___
//           / _ \/ __ `__ \/ __ \/ __/ / ___/ ___/ __ \/ __ \/ _ \
//          /  __/ / / / / / /_/ / /_/ (__  ) /__/ /_/ / /_/ /  __/
//          \___/_/ /_/ /_/\____/\__/_/____/\___/\____/ .___/\___/
//              Audio-visual engine by @lixielabs    /_/
//              Released under the GPLv3 License
//
// ############################################################################
// ## FOREWORD ################################################################
// 
// Welcome to the Emotiscope Engine. This is firmware which: 
// 
// - Logs raw audio data from the microphone into buffers
// - Detects frequencies in the audio using many Goertzel filters
// - Detects the BPM of music
// - Syncronizes to the beats of said music
// - Checks the touch sensors for input
// - Hosts an HTTP server for a web app
// - Talks to that web app over a high speed ws:// connection
// - Stores settings in flash memory
// - Draws custom light show modes to the LEDs which react to music
//   in real-time with a variety of effects
// - Runs the indicator light
// - Runs the screensaver
// - Applies a blue-light filter to the LEDs
// - Applies gamma correction to the LEDs
// - Applies error-diffusion temporal dithering to the LEDs
// - Drives those 128 LEDs with a custom RMT driver at high frame rates
// - Supports over-the-air firmware updates
// - And much more
//
// It's quite a large project, and it's all running on a dual core
// ESP32-S3. (240 MHz CPU with 520 KB of RAM)
//
// This is the main file everything else branches from, and it's where
// the two cores are started. The first core runs the graphics (Core 0)
// and the second core runs the audio and web server (Core 1).
//
// If you enjoy this product or code, please consider supporting me on
// GitHub. I'm a solo developer and I put a lot of time and effort into
// making Emotiscope the best that it can be. Your support helps me
// continue to develop and improve the Emotiscope Engine.
//
//                                  DONATE:
//                                  https://github.com/sponsors/connornishijima
//
//                                               - Connor Nishijima, @lixielabs

// ############################################################################
// ## SOFTWARE VERSION ########################################################

#define SOFTWARE_VERSION_MAJOR ( 1 )
#define SOFTWARE_VERSION_MINOR ( 2 )
#define SOFTWARE_VERSION_PATCH ( 0 )


// ############################################################################
// ## DEPENDENCIES ############################################################

// External dependencies ------------------------------------------------------
//#include <FastLED.h> // .......... You've served me well, but you're not compatible with the 3.0.0-rc1 ESP32 board def yet, and I need the IDF 5.x for this madness to even work. I cobbled my own RMT LED driver for now for non-blocking frame transmission.
#include <PsychicHttp.h> // ........ Handling the web-app HTTP and WS
#include <HTTPClient.h> // ......... Used to make POST requests to the device discovery server
#include <ESPmDNS.h> // ............ Used for "emotiscope.local" domain name
#include <Ticker.h> // ............. For timing functions
#include <DNSServer.h> // .......... Captive portal functionality (not yet working)
#include <Preferences.h> // ........ Storing settings in NVS flash
#include <Update.h> // ............. Inline firmware update library
#include <WiFi.h> // ............... WiFi connection library
#include <esp_dsp.h> // ............ Fast SIMD-style array math
#include <esp_wifi.h> // ........... WiFi, but like - the hardware side of it

// Internal dependencies ------------------------------------------------------
#include "global_defines.h" // ..... Compile-time configuration
#include "hardware_version.h" // ... Baked into the PCB are 4 pins that define the hardware version in binary
#include "types.h" // .............. typedefs for things like CRGBFs
#include "profiler.h" // ........... Developer tools, measures the execution of functions
#include "sliders.h" // ............ Handles sliders that appear in the web app
#include "toggles.h" // ............ Handles toggles that appear in the web app
#include "menu_toggles.h" // ....... Toggles that appear in the main menu
#include "menu_dropdowns.h" // ..... Drop-downs that appear in the main menu
#include "filesystem.h" // ......... LittleFS functions
#include "configuration.h" // ...... Storing and retreiving your settings
#include "utilities.h" // .......... Custom generic math functions
#include "system.h" // ............. Lowest-level firmware functions
#include "led_driver.h" // ......... Low-level LED communication, (ab)uses RMT for non-blocking output
#include "perlin.h" // ............. Perlin noise generator
#include "leds.h" // ............... LED dithering, effects, filters
#include "touch.h" // .............. Handles capacitive touch input
#include "indicator.h" // .......... Little light bulb
#include "ui.h" // ................. Draws UI elements to the LEDs like indicator needles
#include "microphone.h" // ......... For gathering audio chunks from the microphone
#include "vu.h" // ................. Tracks music loudness from moment to moment
#include "goertzel.h" // ........... GDFT or God Damn Fast Transform is implemented here
#include "tempo.h" // .............. Comupation of (and syncronization) to the music tempo
#include "audio_debug.h" // ........ Print audio data over UART
#include "screensaver.h" // ........ Colorful dots play on screen when no audio is present
#include "standby.h" // ............ Handles sleep/wake + animations
#include "light_modes.h" // ........ Definition and handling of light modes
#include "commands.h" // ........... Queuing and parsing of commands recieved
#include "wireless.h" // ........... Communication with your network and the web-app
#include "ota.h" // ................ Over-the-air firmware updates

// Loops ---------------------------------------------------------------------
#include "cpu_core.h" // Audio
#include "gpu_core.h" // Video
#include "web_core.h" // Wireless

// Development Notes
//#include "notes.h"


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
	(void)xTaskCreatePinnedToCore(loop_gpu, "loop_gpu", 4096, NULL, 0, NULL, 0);
}