#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NKEER 50

void afdruk(long reel, struct tms i) {
	long ticks = sysconf(_SC_CLK_TCK);
	printf("TIJDEN %8ld (%ld) : s%3ld u%3ld cs%3ld cu%3ld\n", reel, ticks, i.tms_stime, i.tms_utime, i.tms_cstime, i.tms_cutime);
}

void k1() {
	printf("\tKind 1 gestart (PID: %d, PPID: %d)...\n", getpid(), getppid());
	char *padprog, *prog;
	int r;
	int som=0;

	padprog = getenv("PROG");
	prog = strrchr(padprog, '/') + 1;

	for(int i=0; i < 100000000; i++) {
	}

	r = execl(padprog, prog, 0);

	if(r == -1) {
		printf("\t%d\n", getpid());
	}

	exit(8);
}

void k2sighandler(int sig) {
	printf("\t\tSignaal ontvangen...\n");
	kill(getppid(), SIGUSR2);
}

void k2() {
	printf("\t\tKind 2 gestart (PID: %d, PPID: %d)...\n", getpid(), getppid());
	int teller = 0;
	time_t sec = time(NULL);

	printf("\t\tAantal seconden: %ld\n", sec);
	printf("\t\tHuidige tijd: %s", ctime(&sec));
	signal(SIGUSR1, k2sighandler);

	for(int i=0; i<NKEER; i++) {
		printf("\t\tWachten op signaal (%d)...\n", i);
		pause();
	}

	exit(9);
}

void k3() {
	printf("\t\t\tKind 3 gestart (PID: %d, PPID: %d)...\n", getpid(), getppid());
	char *padprog, *prog;
	int r;
	int som=0;

	padprog = getenv("PROG");
	prog = strrchr(padprog, '/usr/bin/date') + 1;

	r = execl(padprog, prog, 0);

	if(r == -1) {
		printf("\t%d\n", getpid());
	}

	exit(10);
}

void parentSighandler(int sig) {
	printf("Signaal ontvangen...\n");
}

int main(int argc, char *argv[]) {
	printf("Start...\n");

	if(argc != 2) {
		printf("Niet het juiste aantal argumenten!\n");
		exit(-1);
	}

	signal(SIGALRM, exit);
	alarm(atoi(argv[1]));

	struct tms time;
	long timea;
	int k1pid, k2pid, k3pid, r, w;




	timea = times(&time);
	afdruk(timea, time);

	if((k1pid = fork()) == 0) {
		k1();
	}

	if((k2pid = fork()) == 0) {
		k2();
	}

	//Wachten op dood K1
	while(1) {
		r = wait(&w);
		if(r == k1pid) {
			printf("K1 gestorven... (%d %d)\n", r, w);
			break;
		}
	}

	//lus
	signal(SIGUSR2, parentSighandler);
	while(1) {
		//verwittigen k2 (als niet lukt -> afbreken)
		int waarde = kill(k2pid, SIGUSR1);
		printf("Signaal gezonden met waarde: %d\n", waarde);
		if(waarde == -1) {
			break;
		}
		//wachten antwoord k2
		pause();
		
		//k3 maken
		if((k3pid = fork()) == 0) {
			k3();
		}
		//wachten einde k3
		while(1) {
			r = wait(&w);
			if(r == k3pid) {
				printf("K3 gestorven... (%d %d)\n", r, w);
				break;
			}
		}
	}

	timea = times(&time);
	afdruk(timea, time);

	return 0;
}