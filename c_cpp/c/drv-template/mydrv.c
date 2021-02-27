#include "mydrv.h"

#include <stdlib.h>


mydrv_t *mydrv_create()
{
	mydrv_t *ret = (mydrv_t *)calloc(1, sizeof(mydrv_t));
	// callocs
	return ret;
}


void mydrv_free(mydrv_t *mydrv)
{
	// frees
	free(mydrv);
}