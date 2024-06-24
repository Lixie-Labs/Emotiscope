/*
############################################################################
############################################################################
                                 __  _
           ___  ____ ___  ____  / /_(_)_____________  ____  ___
          / _ \/ __ `__ \/ __ \/ __/ / ___/ ___/ __ \/ __ \/ _ \
         /  __/ / / / / / /_/ / /_/ (__  ) /__/ /_/ / /_/ /  __/
         \___/_/ /_/ /_/\____/\__/_/____/\___/\____/ .___/\___/
             Audio-visual engine by @lixielabs    /_/
             Released under the GPLv3 License

############################################################################
## FOREWORD ################################################################
 
Welcome to the Emotiscope Engine. This is firmware which: 
 
- Logs raw audio data from the microphone into buffers
- Detects frequencies in the audio using many Goertzel filters at once
- Detects the BPM of music
- Syncronizes to the beats of said music
- Checks the touch sensors for input
- Talks to the Emotiscope web app over a high speed ws:// connection
- Stores settings in NVS memory
- Draws custom light show modes to the LEDs which react to
  music in real-time with a variety of effects
- Runs the indicator light
- Runs the screensaver
- Applies a blue-light filter to the LEDs
- Applies gamma correction to the LEDs
- Applies error-diffusion temporal dithering to the LEDs
- Drives those 128 LEDs with an RMT driver at high frame rates
- Supports over-the-air firmware updates
- And much more

It's quite a large project, and it's all running on a dual core
ESP32-S3. (240 MHz CPU with 520 KB of RAM)

This is the main file everything else branches from, and it's where
the two cores are started. The first core runs the graphics (Core 0)
and the second core runs the audio and web server (Core 1).

If you enjoy this product or code, please consider supporting me on
GitHub. I'm a solo developer and I put a lot of time and effort into
making Emotiscope the best that it can be. Your support helps me
continue to develop and improve the Emotiscope Engine.

                                  DONATE:
                                  https://github.com/sponsors/connornishijima

                                               - Connor Nishijima, @lixielabs
*/

// ## SOFTWARE VERSION ########################################################

#define SOFTWARE_VERSION_MAJOR ( 2 )
#define SOFTWARE_VERSION_MINOR ( 0 )
#define SOFTWARE_VERSION_PATCH ( 0 )
#define TAG "EE"

// ## DEPENDENCIES ############################################################

// External dependencies ------------------------------------------------------
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_netif.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <sys/param.h>
#include <nvs_flash.h>
#include <nvs.h>

// Internal dependencies ------------------------------------------------------
#include "types.h"
#include "configuration.h"
#include "wireless.h"
#include "system.h"

// Loops ---------------------------------------------------------------------
//#include "cpu_core.h" // Audio
//#include "gpu_core.h" // Video
//#include "web_core.h" // Wireless

// Development Notes
//#include "notes.h"

// ## CODE ####################################################################

// EVERYTHING BEGINS HERE ON BOOT ---------------------------------------------
void app_main(void){
	// (system.h) Initialize all peripherals
	init_system();
}