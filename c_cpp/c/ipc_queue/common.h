
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define COMMON_KEY 		ftok(".", 123)
#define COMMON_PERMS	0666

typedef struct {
    long mesg_type;
    char mesg_text[100];
} mmsg_t;