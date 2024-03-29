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

char keuze;
int k4pid;

void afdruk(long reel, struct tms i)
{
    long ticks;

    ticks = sysconf(_SC_CLK_TCK);
    printf("TIJDEN %8ld (%ld) : s%3d u%3d cs%3d cu%3d\n", reel, ticks, 
                i.tms_stime, i.tms_utime, i.tms_cstime, i.tms_cutime);
}

void k1() {
    printf("\tKind 1 gestart (PID: %d, PPID: %d)...\n", getpid(), getppid());
    char *padprog, *prog;
    int r;
    int som=0;

    padprog = getenv("PROG");
    prog = strrchr(padprog, '/') + 1;

    for(int i=0; i < 10000; i++) {
    }

    r = execl(padprog, prog, 0);

    if(r == -1) {
        printf("\t%d\n", getpid());
    }

    exit(8);
}

void k4() {
    int r;
    
    r = execl("/user/bin/who", "am i", 0);
    
    if(r == -1) {
        printf("\t%d\n", getpid());
    }

    exit(11);
}

void k2sighandler(int sig) {
    printf("\t\tSignaal ontvangen...\n");
    
    //Nieuw kind maken
    if((k4pid = fork()) == 0) {
        k4();
    }
}


void k2actie1(int sig) {
	printf("\t\t\t\tK4 is dood!\n");
}

void k2actie2(int sig) {
	kill(k4pid, SIGKILL);
	printf("\t\t\t\tKind gedood!\n");
}

void k2() {
    printf("\t\tKind 2 gestart (PID: %d, PPID: %d)...\n", getpid(), getppid());
    int teller = 0;
    time_t sec = time(NULL);

    printf("\t\tAantal seconden: %ld\n", sec);
    printf("\t\tHuidige tijd: %s", ctime(&sec));

    for(int i=0; i<NKEER; i++) {
        signal(SIGUSR1, k2sighandler);
	switch (keuze) {
		case 'd':
			break;
		case 'i':
			signal(SIGCLD, SIG_IGN);
			break;
		case 's':
			signal(SIGCLD, k2actie1);
			break;
		case 'S':
			signal(SIGCLD, k2actie2);
			break;
	}

        printf("\t\tWachten op signaal (%d)...\n", i);
        pause();
        usleep(500);
        kill(getppid(), SIGUSR2);
    }

    exit(9);
}

void k3() {
    printf("\tKind 3 gestart (PID: %d, PPID: %d)...\n", getpid(), getppid());
    char *padprog, *prog;
    int r;
    int som=0;

    padprog = getenv("PROG");
    prog = strrchr(padprog, '/') + 1;

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

    if(argc != 3) {
        printf("Niet het juiste aantal argumenten!\n");
        printf("(1) #maxSeconden (2) opties\nd: default\ni: ignore\ns: uitschrijven boodschap\nS: speciefieke actie + zombie-kind laten verdwijnen\n");
        exit(-1);
    }

    signal(SIGALRM, exit);
    alarm(atoi(argv[1]));
    
    keuze = argv[2][0];

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

    r = wait(&w);
    if(r == k1pid) {
        printf("K1 gestorven... (%d %d)\n", r, w);
    }

    //lus
    while(1) {
        //verwittigen k2 (als niet lukt -> afbreken)
        signal(SIGUSR2, parentSighandler);
        int waarde = kill(k2pid, SIGUSR1);
        printf("Signaal gezonden met waarde: %d\n", waarde);
        if(waarde == -1) {
            break;
        }
        
        //wachten antwoord k2
        pause();
        usleep(500);
        
        //k3 maken
        if((k3pid = fork()) == 0) {
            k3();
        }
        //wachten einde k3{
        r = wait(&w);
        if(r == k3pid) {
            printf("K3 gestorven... (%d %d)\n", r, w);
        }

    }

    timea = times(&time);
    afdruk(timea, time);

    return 0;
}
