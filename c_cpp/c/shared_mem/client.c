
#include "common.h"

int main ()
{
	int semid = semget(COMMON_KEY, 1, 0);
	int shmid = shmget(COMMON_KEY, sizeof(ShCtl_t), 0);

	if (semid >= 0 && shmid >= 0) {
		ShCtl_t *msg = (ShCtl_t *)shmat(shmid, 0, 0);
		if (msg) {
			printf("starting\n");
			while (1) {
				usleep(1000);
				if (semctl(semid, 0, GETVAL, 0)) // > 0 - waiting
					continue;
				semctl(semid, 0, SETVAL, 1); // set
				switch (msg->type) {
					case sctEmpty: {
						// nop
					} break;
					case sctData: {
						printf("%d\n", msg->data.d);
						if (msg->data.d > 100) {
							msg->type = sctEnd;
							goto lbl_exit;
						} else {
							msg->type = sctEmpty;
						}
					} break;
					case sctEnd: {
						// nop
					} break;
					default: break;
				}
				semctl(semid, 0, SETVAL, 0); // reset
			}
		lbl_exit:
			shmdt(msg);
		}
	}

	return 0;
}
