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
// C
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Espressif
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_netif.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <esp_random.h>
#include <esp_check.h>
#include <sys/param.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <freertos/event_groups.h>
#include <perfmon.h>

// Peripherals
#include <driver/temperature_sensor.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <driver/touch_pad.h>

// SIMD
#include <esp_dsp.h>
#include <dsps_mem.h>

// Internal dependencies ------------------------------------------------------

// System
#include "global_defines.h"
#include "types.h"
#include "system.h"
#include "configuration.h"
#include "profiler.h"
#include "utilities.h"

// Hardware
#include "microphone.h"
#include "indicator_light.h"
#include "led_driver.h"
#include "touch.h"

// DSP
#include "fft.h"
#include "goertzel.h"
#include "vu.h"
#include "tempo.h"
#include "pitch.h"

// Comms
#include "wireless.h"
#include "packets.h"

// Graphics
#include "perlin.h"
#include "leds.h"
#include "ui.h"
#include "standby.h"
#include "screensaver.h"
#include "light_modes.h"

// Testing
#include "testing.h"

// Cores
#include "cpu_core.h" // Audio/Web
#include "gpu_core.h" // Video

// Development Notes
//#include "notes.h"

// ## CODE ####################################################################

// EVERYTHING BEGINS HERE ON BOOT ---------------------------------------------
void app_main(void){
	// Initialize all peripherals (system.h) 
	init_system();

	//configuration.current_mode.value.u32 = 9;
	configuration.saturation.value.f32 = 0.95;
	configuration.warmth.value.f32 = 0.00;
	configuration.softness.value.f32 = 0.10;
	configuration.speed.value.f32 = 0.75;
	configuration.background.value.f32 = 0.25;
	configuration.color_range.value.f32 = 0.66;
	configuration.reverse_color_range.value.u32 = 0;
	configuration.auto_color_cycle.value.u32 = 0;
	configuration.color_mode.value.u32 = COLOR_MODE_PERLIN;

	// Start the main cores (cpu_core.h, gpu_core.h)
	(void)xTaskCreatePinnedToCore(loop_cpu, "loop_cpu", 4096, NULL, 5, NULL, 0);
	(void)xTaskCreatePinnedToCore(loop_gpu, "loop_gpu", 4096, NULL, 5, NULL, 1);
}