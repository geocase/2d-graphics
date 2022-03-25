#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "fio.h"

char *readFilePathToCStr(const char *path) {
	FILE *f = fopen(path, "rb");
	u64 file_size;
	char *buffer;

	rewind(f);
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	buffer = malloc((file_size + 1) * sizeof(char));

	rewind(f);
	fread(buffer, sizeof(char), file_size, f);
	fclose(f);
	buffer[file_size] = '\0';

	return buffer;
}
