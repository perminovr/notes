
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


char *data;


/// инициализация внутренних переменных либы
void _init()
{
	data = (char *)malloc(5);
	strcpy(data, "data");
}



/// уничтожение внутренних переменных либы
void _fini()
{
	free(data);
}


void printName()
{
	printf("%s", data);
}
