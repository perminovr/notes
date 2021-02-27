#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int file_select(const struct dirent *entry)
{
	if (entry->d_type == DT_REG) return 1;
	return 0;
}

int main()
{
	struct dirent **files;
	int cntfile = scandir("./", &files, file_select, alphasort);
	for (int i=0; i<cntfile; i++) {
		printf("%s\n", files[i]->d_name);
	}
	free(files);
}