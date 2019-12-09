#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

void afdruk(long reel, struct tms i) {
	long ticks = sysconf(_SC_CLK_TCK);
	printf("TIJDEN %8ld (%ld) : s%3ld u%3ld cs%3ld cu%3ld\n", reel, ticks, i.tms_stime, i.tms_utime, i.tms_cstime, i.tms_cutime);
}

void k1() {
	printf("\tKind 1 gestart...\n");
	char *padprog, *prog;
	int r;
	int som=0;

	padprog = getenv("PROG");
	prog = strrchr(padprog, '/') + 1;

	for(int i=0; i < 1000000000000; i++) {
		som+=1;
		for(int i=0; i < 1000000000000; i++) {
			som+=1;
			for(int i=0; i < 1000000000000; i++) {
				som+=1;
			}
		}
	}

	r = execl(padprog, prog, 0);

	if(r == -1) {
		printf("\t%d\n", getpid());
	}

	exit(8);
}

void k2() {
	printf("\t\tKind 2 gestart...\n");
	time_t sec = time(NULL);

	printf("\t\tAantal seconden: %ld\n", sec);
	printf("\t\tHuidige tijd: %s", ctime(&sec));

	pause();

}

int main(int argc, char *argv[]) {
	printf("Start...\n");

	struct tms time;
	long timea;
	int k1pid, k2pid, r, w;


	timea = times(&time);
	afdruk(timea, time);

	if((k1pid = fork()) == 0) {
		k1();
	}

	if((k2pid = fork()) == 0) {
		k2();
	}

	//Automatisch sterven van K1
	r = wait(&w);
	printf("k1: %d, %d\n",r, w);

	//Doden van K2
	kill(k2pid, SIGUSR1);
	r = wait(&w);
	printf("k2: %d, %d\n",r, w);

	timea = times(&time);
	afdruk(timea, time);

	return 0;
}