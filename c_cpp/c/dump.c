static void DumpHex(const void* data, size_t size)
{
    #define DUMPHEX_WIDTH 16
	char dump[DUMPHEX_WIDTH*16];
	size_t it = 0;

	char ascii[DUMPHEX_WIDTH+1];
	size_t i, j;
	ascii[DUMPHEX_WIDTH] = 0;

	for (i = 0; i < size; ++i) {
		sprintf(dump + it, "%02X ", ((uint8_t*)data)[i]);
		it += strlen(dump + it);
		if (((uint8_t*)data)[i] >= ' ' && ((uint8_t*)data)[i] <= '~') {
			ascii[i % DUMPHEX_WIDTH] = ((uint8_t*)data)[i];
		} else {
			ascii[i % DUMPHEX_WIDTH] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			sprintf(dump + it, " ");
			it += strlen(dump + it);
			if ((i+1) % DUMPHEX_WIDTH == 0) {
				sprintf(dump + it, "|  %s", ascii);
				it += strlen(dump + it);
				dump[it] = 0;
				printf("%s\n", dump);
				it = 0;
			} else if (i+1 == size) {
				ascii[(i+1) % DUMPHEX_WIDTH] = '\0';
				if ((i+1) % DUMPHEX_WIDTH <= 8) {
					sprintf(dump + it, " ");
					it += strlen(dump + it);
				}
				for (j = (i+1) % DUMPHEX_WIDTH; j < DUMPHEX_WIDTH; ++j) {
					sprintf(dump + it, "   ");
					it += strlen(dump + it);
				}
				sprintf(dump + it, "|  %s", ascii);
				it += strlen(dump + it);
				dump[it] = 0;
				printf("%s\n", dump);
				it = 0;
			}
		}
	}
}
