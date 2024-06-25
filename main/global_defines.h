/*
-----------------------------------------------------------------------------
--- EMOTISCOPE ENGINE -------------------------------------------------------

	global_defines.h
		- Values that are widely used throughout Emotiscope Engine are
		  defined here
   
-----------------------------------------------------------------------------
*/

// It won't void any kind of stupid warranty, but things *may* break at this point if you change this number.
#define NUM_LEDS ( 128 ) // MUST be divisible by 2

// Number of Goertzel instances running in parallel
#define NUM_FREQS ( 64 ) 

// Number of times per second "novelty" is logged
#define NOVELTY_LOG_HZ (50)

// 50 FPS for 20.48 seconds
#define NOVELTY_HISTORY_LENGTH (1024)

// TEMPO_LOW to TEMPO_HIGH
#define NUM_TEMPI (128)

// BPM range
#define TEMPO_LOW (60)
#define TEMPO_HIGH (TEMPO_LOW + NUM_TEMPI)

// How far forward or back in time the beat phase is shifted
#define BEAT_SHIFT_PERCENT (0.00)

// Set later by physical traces on the PCB
uint8_t HARDWARE_VERSION = 0;

// WiFi credentials
char wifi_ssid[128] = { 0 };
char wifi_pass[128] = { 0 };

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))