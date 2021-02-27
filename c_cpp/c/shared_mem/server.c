
#include "common.h"

int main ()
{
	int semid = semget(COMMON_KEY, 1, COMMON_PERMS | IPC_CREAT);
	int shmid = shmget(COMMON_KEY, sizeof(ShCtl_t), COMMON_PERMS | IPC_CREAT);

	int cnt = 0;

	if (semid >= 0 && shmid >= 0) {
		ShCtl_t *msg = (ShCtl_t *)shmat(shmid, 0, 0);
		if (msg) {
			printf("starting\n");
			semctl(semid, 0, SETVAL, 0);
			bzero(msg, sizeof(ShCtl_t));
			while (1) {
				usleep(1000);
				if (semctl(semid, 0, GETVAL, 0)) // > 0 - waiting
					continue;
				semctl(semid, 0, SETVAL, 1); // set
				switch (msg->type) {
					case sctEmpty: {
						cnt++;
						msg->data.d = cnt;
						msg->type = sctData;
					} break;
					case sctData: {
						// nop
					} break;
					case sctEnd: {
						goto lbl_exit;
					} break;
					default: break;
				}
				semctl(semid, 0, SETVAL, 0); // reset
			}
		lbl_exit:
			semctl(semid, 0, IPC_RMID, NULL);
			shmdt(msg);
			shmctl(shmid, IPC_RMID, NULL);
		}
	}
	return 0;
}