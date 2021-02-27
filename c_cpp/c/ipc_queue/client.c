#include "common.h"

int main()
{
	mmsg_t msg;
	key_t key = COMMON_KEY;
	int msgid = msgget(key, IPC_CREAT | COMMON_PERMS);

	msg.mesg_type = 1;
	printf("Write Data : ");
	gets(msg.mesg_text);

	// msgsnd to send message
	msgsnd(msgid, &msg, sizeof(msg), 0);

	// display the message
	printf("Data send is : %s \n", msg.mesg_text);

	return 0;
}