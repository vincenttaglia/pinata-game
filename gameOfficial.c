/*--------------------------------------------------------------------------*
 *----									----*
 *----		gameOfficial.c						----*
 *----									----*
 *----	    This program controls version 2.0 of the pinata-whacking	----*
 *----	simulator.							----*
 *----									----*
 *----	----	----	----	----	----	----	----	----	----*
 *----									----*
 *----	Version 2.0		2017 September 27	Joseph Phillips	----*
 *----									----*
 *--------------------------------------------------------------------------*/

/*----*
 *----*		Common include sequence:
 *----*/
#include	<stdlib.h>
#include	<stdio.h>
#include	<sys/types.h>
#include    <sys/wait.h>
#include	<signal.h>
#include	<string.h>
#include	<unistd.h>


/*----*
 *----*		Declaration of constants:
 *----*/
#define		LINE_LEN		16
#define		PINATA_PROG_NAME	"pinata"
#define		CHILD_PROG_NAME		"child"


/*----*
 *----*		Definition of global vars:
 *----*/
int		shouldRun		= 1;
int isWaitingForTurn;

pid_t*		childPidArray;
pid_t		pinataPid;


/*----*
 *----*		Definition of global fncs:
 *----*/

/*  PURPOSE:  To change the global state so that the program knows both that
 *	the current child process has finished its turn, and that it the game
 *	is over (that child won).  Ignores parameters.  No return value.
 */
void	turnOverStopGame
		(int		sig,
		 siginfo_t*	info,
		 void*		data
		)
{
  shouldRun		= 0;
  isWaitingForTurn	= 0;
}


/*  PURPOSE:  To change the global state so that the program knows that the
 *	current child process has finished its turn, but that it the game is
 *	not yet over (that child lost).  Ignores parameters.  No return value.
 */
void	turnOver(int		sig,
		 siginfo_t*	info,
		 void*		data
		)
{
  isWaitingForTurn	= 0;
}


/*  PURPOSE:  To reap all child processes that have already finished.  Ignores
 *	parameters.  No return value.
 */
void	child	(int		sig,
		 siginfo_t*	info,
		 void*		data
		)
{
  int	status;
  pid_t	finishedId;
  pid_t tempId;

  while((finishedId = waitpid(getpid(), &status, WNOHANG)) > 0){
	  
  }
  printf("%d has finished with exit code %d.\n", info->si_pid, WEXITSTATUS(status));
}



/*  PURPOSE:  To simulate the pinata-whacking game.  Ignores command line
 *	parameters.  Returns EXIT_SUCCESS to OS on success or EXIT_FAILURE
 *	otherwise.
 */
int	main	()
{
  //  I.  Application validity check:

  //  II.  Do simulation:
  //  II.A.  Get simulation parameters:
  int		numChildren;
  char		line[LINE_LEN];

  //  II.A.1.  Get 'numChildren' (must be greater than or equal to 1):
  printf("Number of children? ");
  scanf ("%d",&numChildren);
  while(numChildren < 1){
	printf("Number of children? ");
	scanf ("%d",&numChildren);
  }


  //  II.B.  Prepare game:
  //  II.B.1.  Initialize 'childPidArray':
  childPidArray	      = (pid_t*)calloc(numChildren,sizeof(pid_t));

  //  II.B.2.  Install signal handlers:
  struct sigaction sa;
  memset(&sa,'\0',sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask );
  sa.sa_flags= SA_SIGINFO | SA_RESTART;
  sa.sa_sigaction = turnOver;
  
  struct sigaction sb;
  memset(&sb,'\0',sizeof(struct sigaction));
  sigemptyset(&sb.sa_mask );
  sb.sa_flags= SA_SIGINFO | SA_RESTART;
  sb.sa_sigaction = turnOverStopGame;
  
  struct sigaction sc;
  memset(&sc,'\0',sizeof(struct sigaction));
  sigemptyset(&sc.sa_mask );
  sc.sa_flags= SA_SIGINFO | SA_RESTART;
  sc.sa_sigaction = child;
  
  sigaction(SIGCHLD, &sc, NULL);
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sb, NULL);
  
  char *cwd;
  cwd = malloc(256);
  
  if(getcwd(cwd, 256)== NULL){
	  printf("cwd error\n");
  }
  cwd = strcat(cwd,"/");

  //  II.C.  Launch child processes:
  //  II.C.1.  Launch pinata process:
  pinataPid = fork();

  if  (pinataPid == -1)
  {
    fprintf(stderr,"Your OS is being fork()-bombed! :(\n");
    exit(EXIT_FAILURE);
  }

  if  (pinataPid == 0)
  {
    execl(strcat(cwd,PINATA_PROG_NAME), PINATA_PROG_NAME, NULL);
    fprintf(stderr,"Could not find program %s! :(\n",PINATA_PROG_NAME);
    exit(EXIT_FAILURE);
  }

  //  II.C.2.  Launch pinata-whacking child process(es):
  int	i;

  for  (i = 0;  i < numChildren;  i++)
  {
    childPidArray[i] = fork();

    if  (childPidArray[i] == -1)
    {
      fprintf(stderr,"Your OS is being fork()-bombed! :(\n");
      exit(EXIT_FAILURE);
    }

    if  (childPidArray[i] == 0)
    {
      char	numText[LINE_LEN];

      snprintf(line,LINE_LEN,"%d",pinataPid);
      snprintf(numText,LINE_LEN,"%d",i);
      execl(strcat(cwd,CHILD_PROG_NAME), CHILD_PROG_NAME, line, numText, NULL);
      fprintf(stderr,"Could not find program %s! :(\n",CHILD_PROG_NAME);
      exit(EXIT_FAILURE);
    }

  }

  //  II.D.  Play game:
  //  II.D.1.  Wait a sec' for all child processes to compose themselves:
  sleep(1);

  //  II.D.2.  Each iteration tells does one turn of one pinata-whacking
  //  	       child process:
  int	currentChild	= 0;
  //printf("Child PID: %d\n", childPidArray[currentChild]);
  while  (1)
  {
	
    printf("Child %d's turn:\n",currentChild);
    isWaitingForTurn = 1;
    kill(childPidArray[currentChild], SIGUSR1);
    
    while  (isWaitingForTurn)
      sleep(3);

    if  ( !shouldRun )
      break;

    currentChild++;

    if  (currentChild >= numChildren)
      currentChild = 0;
  }

  printf("Child %d won!\n",currentChild);

  //  II.E.  Clean up after game:
  //  II.E.1.  Tell all processes to end themselves:
  for  (currentChild = 0;  currentChild < numChildren;  currentChild++)
  {
	kill(childPidArray[currentChild], SIGINT);
    sleep(1);
  }

  kill(pinataPid, SIGINT);
  sleep(1);

  //  II.E.2.  Clean up memory:
  free(childPidArray);
  free(cwd);

  //  III.  Finished:
  return(EXIT_SUCCESS);
}
