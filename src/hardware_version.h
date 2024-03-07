#define VER1_PIN ( 12 )
#define VER2_PIN ( 13 )
#define VER3_PIN ( 14 )
#define VER4_PIN ( 15 )

void read_hardware_version_pins(){
	uint8_t ver1 = !digitalRead(VER1_PIN);
	uint8_t ver2 = !digitalRead(VER2_PIN);
	uint8_t ver3 = !digitalRead(VER3_PIN);
	uint8_t ver4 = !digitalRead(VER4_PIN);

	printf("PIN READING: %d %d %d %d\n", ver1, ver2, ver3, ver4);

	HARDWARE_VERSION = (ver1 << 3) | (ver2 << 2) | (ver3 << 1) | (ver4 << 0);
}

void init_hardware_version_pins(){
	pinMode(VER1_PIN, INPUT_PULLUP);
	pinMode(VER2_PIN, INPUT_PULLUP);
	pinMode(VER3_PIN, INPUT_PULLUP);
	pinMode(VER4_PIN, INPUT_PULLUP);

	read_hardware_version_pins();
}