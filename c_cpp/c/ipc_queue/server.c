#include "common.h"

int main()
{
	mmsg_t msg;
	key_t key = COMMON_KEY;
	int msgid = msgget(key, IPC_CREAT | COMMON_PERMS);

	// msgrcv to receive message
	msgrcv(msgid, &msg, sizeof(msg), 1, 0);

	// display the message
	printf("Data Received is : %s \n", msg.mesg_text);

	// to destroy the message queue
	msgctl(msgid, IPC_RMID, NULL);

	return 0;
}