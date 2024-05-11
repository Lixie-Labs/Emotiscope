enum update_type {
	FLASH,
	FILESYSTEM
};

HTTPClient http;

bool update_running = false;

String latest_version;

bool update_firmware(const char* url, update_type type, int16_t client_slot) {
	http.begin(url);

	int32_t http_code = http.GET();
	printf("HTTP CODE: %li\n", http_code);
	if (http_code == HTTP_CODE_OK) {
		int content_length = http.getSize();
		if (content_length > 0) {
			bool can_begin = false;
			if (type == FLASH) {
				can_begin = Update.begin(content_length, U_FLASH);
			} else if (type == FILESYSTEM) {
				can_begin = Update.begin(content_length, U_SPIFFS);
			}

			if (can_begin) {
				WiFiClient* stream = http.getStreamPtr();
				Update.onProgress([type, client_slot](size_t progress, size_t total) {
					float percent = (progress / (total / 100));

					char update_message[64];
					memset(update_message, 0, 64);
					if (type == FLASH) {
						printf("Firmware update progress: %d%%\r", (int)percent);
						snprintf(update_message, 64, "ota_firmware_progress|%d", (int)percent);
					} else if (type == FILESYSTEM) {
						printf("Filesystem update progress: %d%%\r", (int)percent);
						snprintf(update_message, 64, "ota_filesystem_progress|%d", (int)percent);
					}

					transmit_to_client_in_slot(update_message, client_slot);
				});

				size_t written = 0;
				size_t chunk_size = 256; // Adjust the chunk size as needed
				uint8_t buffer[chunk_size];
				while (http.connected() && (written < content_length)) {
					size_t bytes_read = stream->readBytes(buffer, chunk_size);
					if (bytes_read > 0) {
						Update.write(buffer, bytes_read);
						written += bytes_read;
					}
				}

				if (written == content_length) {
					if (Update.end()) {
						printf("\nFirmware updated successfully.\n");
						return true;
					} else {
						printf("\nFirmware update failed.\n");
					}
				} else {
					printf("\nFirmware size mismatch.\n");
				}
			} else {
				printf("Not enough space to update firmware.\n");
			}
		} else {
			printf("Invalid firmware size.\n");
		}
	} else {
		printf("Failed to download firmware.\n");
	}
	http.end();
	return false;
}

bool check_update(){
	http.begin("https://emotiscope.rocks/latest_version.txt");
	int http_code = http.GET();
	if (http_code == HTTP_CODE_OK) {
		latest_version = http.getString();
		latest_version.trim(); // Remove trailing newline
		http.end();

		// Assemble a char array from the current version numbers using snprintf
		char current_version[16];
		memset(current_version, 0, 16);
		snprintf(current_version, 16, "%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH);

		printf("CURRENT VERSION: %s\n", current_version);
		printf("LATEST VERSION: %s\n", latest_version.c_str());

		// Compare the latest version with the current version
		if (strcmp(latest_version.c_str(), current_version) == 0) {
			printf("FIRMWARE UP TO DATE\n");
			return false;
		} else {
			printf("FIRMWARE OUT OF DATE\n");
			return true;
		}
	}
	else {
		printf("FAILED TO FETCH LATEST VERSION NUMBER\n");
		printf("HTTP CODE: %d\n", http_code);

		return false;
	}
}

void perform_update(int16_t client_slot){
	update_running = true;

	// Update firmware
	String new_firmware = "https://app.emotiscope.rocks/versions/" + latest_version + "/firmware.bin";
	printf("DOWNLOADING/INSTALLING FIRMWARE UPDATE\n");
	update_firmware(new_firmware.c_str(), FLASH, client_slot);

	// Update filesystem
	String new_filesystem = "https://app.emotiscope.rocks/versions/" + latest_version + "/littlefs.bin";
	printf("DOWNLOADING/INSTALLING FILESYSTEM UPDATE\n");
	update_firmware(new_filesystem.c_str(), FILESYSTEM, client_slot);

	printf("UPDATE COMPLETE\n");
	delay(1000);
	printf("REBOOTING...\n");
	delay(100);
	ESP.restart();
}