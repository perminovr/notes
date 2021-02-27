
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

static int extractNumbers(const char *string, int *array, int arraySize)
{
	int ret = 0;
	char buffer[256];
	snprintf(buffer, sizeof(buffer)-1, "%s.", string);
	buffer[sizeof(buffer)-2] = '.';
	buffer[sizeof(buffer)-1] = 0;
	int strSize = strlen(buffer);

	int i = 0;
	int findForEnd = 0;
	char *start = 0;
	for (; i < strSize && ret != arraySize; ++i) {
		if (isdigit(buffer[i])) { // for first numeral
			if (!start) {
				start = buffer+i;
			}
		} else { // first alpha
			if (start) { // after numeral(s)
				int value = atoi(start);
				if (findForEnd) { // we are finding for end of range
					findForEnd = 0;
					if (ret > 0) { // paranoid if
						for (int j = (int)array[ret-1]+1; j < value && ret != arraySize; ++j) {
							array[ret++] = j;
						}
					}
				}
				if (ret != arraySize) {
					array[ret++] = value;
					start = 0;
				}
			}
			if (ret > 0) { // range
				// hyphen or two dots
				if (buffer[i] == '-' || (i > 0 && buffer[i-1] == '.' && buffer[i] == '.')) {
					findForEnd = 1; // find for end of range
				}
			}
		}
	}
	return ret;
}

int main()
{
	#define NUMBERS_SIZE 32
	int numbers[NUMBERS_SIZE];
	const char *str = "-hfjk;k[p]fg 101, 199 102, 103 - 112, 110 233 .. 255 hfjk;k[p]fg";
	int res = extractNumbers(str, numbers, NUMBERS_SIZE);
	for (int i = 0; i < res; ++i) {
		printf("%hhu\n", numbers[i]);
	}
	return 0;
}