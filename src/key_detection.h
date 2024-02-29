uint16_t musical_keys[24] = {
    // Major scales
    0b000010101101, // C major
    0b000011010110, // C# major
    0b000010110101, // D major
    0b000010101011, // Eb major
    0b000010110101, // E major
    0b000010101101, // F major
    0b000011010101, // F# major
    0b000010101110, // G major
    0b000011010101, // G# major
    0b000010101011, // A major
    0b000010110110, // Bb major
    0b000010101101, // B major

    // Minor scales
    0b000010110110, // C minor
    0b000010101101, // C# minor
    0b000011011010, // D minor
    0b000010110101, // Eb minor
    0b000010101011, // E minor
    0b000010110110, // F minor
    0b000010101101, // F# minor
    0b000011010110, // G minor
    0b000010101101, // G# minor
    0b000011010101, // A minor
    0b000010101110, // Bb minor
    0b000011010101  // B minor
};

float scale_confidence[24] = {
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, // Major
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0  // Minor
};

void run_key_detection(){
	float max_confidence = 0.01;
	for(uint16_t i = 0; i < 24; i++){
		float total_difference = 0.00000001;

		for(uint16_t n = 0; n < 12; n++){
			float scale_note_value = (float)bitRead( musical_keys[i], n );
			float chromagram_value = chromagram[n];

			total_difference += fabs(scale_note_value - chromagram_value);
		}

		scale_confidence[i] = (total_difference / 5.0);

		max_confidence = max(max_confidence, scale_confidence[i]);
	}

	float auto_scale = 1.0 / max_confidence;

	for(uint16_t i = 0; i < 24; i++){
		float confidence = scale_confidence[i] * auto_scale;
		CRGBF color = hsv(0.0, 1.0, confidence*confidence);

		//leds[(i << 1) + 0] = color;
		//leds[(i << 1) + 1] = color;
	}
}