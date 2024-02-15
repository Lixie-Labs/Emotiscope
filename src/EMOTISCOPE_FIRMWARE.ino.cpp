# 1 "C:\\Users\\conno\\AppData\\Local\\Temp\\tmpw418d7bx"
#include <Arduino.h>
# 1 "C:/Users/conno/Emotiscope/src/EMOTISCOPE_FIRMWARE.ino"
# 14 "C:/Users/conno/Emotiscope/src/EMOTISCOPE_FIRMWARE.ino"
#define VERSION_MAJOR ( 1 )
#define VERSION_MINOR ( 0 )
#define VERSION_PATCH ( 0 )






#include <PsychicHttp.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <Ticker.h>
#include <WiFi.h>
#include <esp_dsp.h>
#include <esp_wifi.h>


#include "secrets.h"
#include "global_defines.h"
#include "types.h"
#include "profiler.h"
#include "sliders.h"
#include "toggles.h"
#include "filesystem.h"
#include "configuration.h"
#include "utilities.h"
#include "system.h"
#include "touch.h"
#include "led_driver.h"
#include "leds.h"
#include "ui.h"
#include "microphone.h"
#include "vu.h"
#include "goertzel.h"
#include "tempo.h"
#include "audio_debug.h"
#include "screensaver.h"
#include "standby.h"
#include "lightshow_modes.h"
#include "commands.h"
#include "wireless.h"


#include "cpu_core.h"
#include "gpu_core.h"
#include "web_core.h"


#include "notes.h"


#include "secrets.h"
void loop();
void loop_gpu(void *param);
void setup();
#line 72 "C:/Users/conno/Emotiscope/src/EMOTISCOPE_FIRMWARE.ino"
void loop() {
 run_cpu();
 run_web();
}


void loop_gpu(void *param) {
 for (;;) {
  run_gpu();
 }
}


void setup() {

 init_system();


 (void)xTaskCreatePinnedToCore(loop_gpu, "loop_gpu", 8192, NULL, 0, NULL, 0);
}