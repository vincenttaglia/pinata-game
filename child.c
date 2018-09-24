#include	<stdlib.h>
#include	<stdio.h>
#include	<sys/types.h>
#include    <sys/wait.h>
#include	<signal.h>
#include	<string.h>
#include	<unistd.h>

int shouldRun = 1;
pid_t pinataPid;
int childNum;

void usedTurn(int sig, siginfo_t* info, void* data){
	if(getppid() == info->si_pid){
		printf("Child %d: I'm going to whack at it!\n", childNum);
		kill(pinataPid, SIGUSR1);
	}else{
		printf("Child %d: A swing and a miss!\n", childNum);
		kill(getppid(), SIGUSR1);
	}
}

void endPinata(int sig, siginfo_t* info, void* data){
	printf("Child %d stopping.\n", childNum);
	shouldRun = 0;
	fflush(stdout);
}

void wonTurn(int sig, siginfo_t* info, void* data){
	printf("Oh yea! All that candy is MINE baby!\n");
	kill(getppid(), SIGUSR2);
}

int main(int argc, char *argv[]){
	
	if(argc != 3){
		fprintf(stderr,"Usage: %s <pinataPid> <childNum>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	long long int pid = strtoll(argv[1], NULL, 0);
	pinataPid = (pid_t) pid;
	long int num = strtol(argv[2], NULL, 0);
	childNum = (int) num;
	srand(getpid());
	
	struct sigaction sa;
    memset(&sa,'\0',sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask );
    sa.sa_flags= SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = endPinata;
    
    struct sigaction sb;
    memset(&sb,'\0',sizeof(struct sigaction));
    sigemptyset(&sb.sa_mask );
    sb.sa_flags= SA_SIGINFO | SA_RESTART;
    sb.sa_sigaction = usedTurn;
    
    struct sigaction sc;
    memset(&sc,'\0',sizeof(struct sigaction));
    sigemptyset(&sc.sa_mask );
    sc.sa_flags= SA_SIGINFO | SA_RESTART;
    sc.sa_sigaction = wonTurn;
    
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sb, NULL);
	sigaction(SIGUSR2, &sc, NULL);
	
	while  (shouldRun)
		sleep(1);
	exit(EXIT_SUCCESS);
}

