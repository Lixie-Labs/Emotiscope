/*
-----------------------------------------------------------------------------
--- EMOTISCOPE ENGINE -------------------------------------------------------

	hardware_version.h
		- Reads the Hardware Version from the PCB
   
-----------------------------------------------------------------------------
*/

// Pins connected to physical traces on the PCB that define the hardware version in binary
#define VER1_PIN ( 12 )
#define VER2_PIN ( 13 )
#define VER3_PIN ( 14 )
#define VER4_PIN ( 15 )

// Initialize the hardware version pins and read them into a global variable
void init_hardware_version_pins(){
	// Disconnected = HIGH, Connected = LOW
	pinMode(VER1_PIN, INPUT_PULLUP);
	pinMode(VER2_PIN, INPUT_PULLUP);
	pinMode(VER3_PIN, INPUT_PULLUP);
	pinMode(VER4_PIN, INPUT_PULLUP);

	// Invert the readings: Disconnected = 0, Connected = 1
	uint8_t ver1 = !digitalRead(VER1_PIN);
	uint8_t ver2 = !digitalRead(VER2_PIN);
	uint8_t ver3 = !digitalRead(VER3_PIN);
	uint8_t ver4 = !digitalRead(VER4_PIN);

	// Combine the readings into a single byte
	HARDWARE_VERSION = (ver1 << 3) | (ver2 << 2) | (ver3 << 1) | (ver4 << 0);

	// Print the hardware version to the serial console
	printf("HARDWARE VERSION: %d (PINS: %d %d %d %d)\n", HARDWARE_VERSION, ver1, ver2, ver3, ver4);
}