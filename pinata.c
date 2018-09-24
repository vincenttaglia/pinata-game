#include	<stdlib.h>
#include	<stdio.h>
#include	<sys/types.h>
#include    <sys/wait.h>
#include	<signal.h>
#include	<string.h>
#include	<unistd.h>

int shouldRun;

void useTurn(int sig, siginfo_t* info, void* data){
	int	isBroken	= (rand() % 20) == 19;
	int	signalToSend	= (isBroken ? SIGUSR2 : SIGUSR1);
	if(signalToSend == SIGUSR2){	
		printf("Pinata opened!\n");
	}
	kill(info->si_pid, signalToSend);
}

void endPinata(int sig, siginfo_t* info, void* data){
	printf("Pinata stopping.\n");
	shouldRun = 0;
	fflush(stdout);
}

void main(){
	shouldRun = 1;
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
    sb.sa_sigaction = useTurn;
    
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sb, NULL);
	while  (shouldRun)
		sleep(1);
	exit(EXIT_SUCCESS);
}

