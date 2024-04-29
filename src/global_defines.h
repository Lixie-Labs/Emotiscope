// It won't void any kind of stupid warranty, but things will *definitely* break at this point if you change this number.
#define NUM_LEDS ( 128 )

#define NUM_FREQS ( 64 ) // Number of Goertzel instances running in parallel
#define MAX_WEBSOCKET_CLIENTS ( 4 ) // Max simultaneous remote controls allowed at one time

#define NOVELTY_HISTORY_LENGTH (1024)  // 50 FPS for 20.48 seconds
#define NOVELTY_LOG_HZ (50)

#define NUM_TEMPI (96) // TEMPO_LOW to TEMPO_HIGH
#define BEAT_SHIFT_PERCENT (0.16)

#define TEMPO_LOW (32) // BPM
#define TEMPO_HIGH (TEMPO_LOW + NUM_TEMPI)

uint8_t HARDWARE_VERSION = 0;

char wifi_ssid[64] = { 0 };
char wifi_pass[64] = { 0 };

const char* wtf_error_message_header = "^&*@!#^$*WTF?!(%$*&@@^@^$^#&$^ \nWTF ERROR: How the hell did you get here:";
const char* wtf_error_message_tail   = "--------------------------------------------------------------------------";

inline void wtf_error(){
	printf("%s\n", wtf_error_message_header);
	printf("FILE: %s *NEAR* THIS LINE: %li\n", __FILE__, __LINE__ ); // If I inline this function, will this line number roughly match where this function was called from?
	printf("%s\n", wtf_error_message_tail);
}