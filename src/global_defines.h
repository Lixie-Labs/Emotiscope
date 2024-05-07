/*
-----------------------------------------------------------------------------
--- EMOTISCOPE ENGINE -------------------------------------------------------

	global_defines.h
		- Values that are widely used throughout Emotiscope Engine are
		  defined here
   
-----------------------------------------------------------------------------
*/

// It won't void any kind of stupid warranty, but things will *definitely* break at this point if you change this number.
#define NUM_LEDS ( 128 )

// Number of Goertzel instances running in parallel
#define NUM_FREQS ( 64 ) 

// How many characters a command can have
#define MAX_COMMAND_LENGTH (256) 

// Max simultaneous remote controls allowed at one time
#define MAX_WEBSOCKET_CLIENTS ( 4 )

// Number of times per second "novelty" is logged
#define NOVELTY_LOG_HZ (50)

// 50 FPS for 20.48 seconds
#define NOVELTY_HISTORY_LENGTH (1024)

// TEMPO_LOW to TEMPO_HIGH
#define NUM_TEMPI (96)

// BPM range
#define TEMPO_LOW (48)
#define TEMPO_HIGH (TEMPO_LOW + NUM_TEMPI)

// How far forward or back in time the beat phase is shifted
#define BEAT_SHIFT_PERCENT (0.16)

// Set later by physical traces on the PCB
uint8_t HARDWARE_VERSION = 0;

// WiFi credentials
char wifi_ssid[64] = { 0 };
char wifi_pass[64] = { 0 };

// WTF ERRORs are used when the program enters a state that should be impossible -------------------
const char* wtf_error_message_header = "^&*@!#^$*WTF?!(%$*&@@^@^$^#&$^ \nWTF ERROR: How the hell did you get here:";
const char* wtf_error_message_tail   = "--------------------------------------------------------------------------";

inline void wtf_error(){
	printf("%s\n", wtf_error_message_header);
	printf("FILE: %s *NEAR* THIS LINE: %d\n", __FILE__, __LINE__ ); // If I inline this function, will this line number roughly match where this function was called from?
	printf("%s\n", wtf_error_message_tail);
}
// -------------------------------------------------------------------------------------------------
