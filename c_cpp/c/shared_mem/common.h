#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define COMMON_KEY 		ftok(".", 123)
#define COMMON_PERMS	0666

typedef enum {
	sctEmpty,
	sctData,
	sctEnd,
} ShCtlType_e;

typedef struct {
	int d;
} ShData_t;

typedef struct {
	ShCtlType_e type;
	ShData_t data;
} ShCtl_t;
