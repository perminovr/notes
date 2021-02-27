#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#define PARENT_PIPE	0	// read
#define CHILD_PIPE	1	// write

// void sighandler(int signo)
// {
//     if (signo == SIGCHLD) {
// 		int status;
// 		(void)wait(&status);
//     }
// }

pid_t systemp(int *procEndPipe, const char *program, ...)
{
	pid_t ret;
	int pidPipe[2];
	int rc = pipe(pidPipe);

	// signal(SIGCHLD, sighandler);

	switch ( ret = fork() ) {
	case -1: // error
		break;
	case 0: // child
		ret = getpid(); // child pid
		// send child pid to parent
		if (write(pidPipe[CHILD_PIPE], &ret, sizeof(pid_t)) == sizeof(pid_t)) {
			rc = execl("/bin/sh", "/bin/sh", "-c", "sleep 100", NULL);
			if (procEndPipe)
				rc = write(procEndPipe[CHILD_PIPE], &ret, sizeof(pid_t)); // cmd done -> send
			(void)rc;
		}
		close(pidPipe[CHILD_PIPE]);
		exit(0);
		break;
	default: // self
		// waiting for child pid
		if (read(pidPipe[PARENT_PIPE], &ret, sizeof(pid_t)) != sizeof(pid_t)) { // blocking
			ret = -1;
		}
		close(pidPipe[PARENT_PIPE]);
		break;
	}

	return ret;
}

int main()
{
	int rc;
	int procEndPipe[2];
	pid_t spid;

	rc = pipe(procEndPipe);
	spid = systemp(procEndPipe, "sh");

	// read(procEndPipe[PARENT_PIPE], &rc, sizeof(int)); // cmd done -> send
	printf("spid = %d, rc = %d\n", spid, rc);
}