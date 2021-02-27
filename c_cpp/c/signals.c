#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

void my_sa_sigaction(int sig, siginfo_t *info, void *ignored)
{
	printf("--- my_sa_sigaction %d %p\n", sig, info);
	exit(0);
}

int main()
{
	struct sigaction siga;
	siga.sa_sigaction = my_sa_sigaction;
	siga.sa_flags = SA_SIGINFO;
	sigemptyset(&siga.sa_mask);

	sigaction(SIGILL, &siga, 0);
	sigaction(SIGINT, &siga, 0);
	sigaction(SIGABRT, &siga, 0);
	sigaction(SIGFPE, &siga, 0);
	sigaction(SIGSEGV, &siga, 0);
	sigaction(SIGTERM, &siga, 0);
	sigaction(SIGHUP, &siga, 0);
	sigaction(SIGKILL, &siga, 0);

	int i = 1/0;
	return i;
}
