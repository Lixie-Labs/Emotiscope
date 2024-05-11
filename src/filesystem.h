#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

void init_filesystem() {
	if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
		printf("FS | An Error has occurred while mounting LittleFS!\n");
	}
	else {
		printf("FS | LittleFS mounted successfully!\n");
	}
}

void list_dir(File dir, uint8_t levels) {
	while (File file = dir.openNextFile()) {
		for (uint8_t i = 0; i < levels; i++) {
			printf("\t");
		}
		printf("%s", file.name());
		if (file.isDirectory()) {
			printf("/\n");
			list_dir(file, levels + 1);
		}
		else {
			printf("\t\t%zu\n", file.size());
		}
		file.close();
	}
}

void print_filesystem() {
	printf("FS | Filesystem printout V -----------------\n");
	File root = LittleFS.open("/");
	list_dir(root, 0);
	root.close();
	printf("FS | Filesystem printout ^ -----------------\n");
}