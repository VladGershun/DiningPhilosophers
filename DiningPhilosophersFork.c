/* 
* Creator: Vlad Gershun
* Date: 10/20/2021
* Class: CS 360
* Project: HW 5 - Create a program to solve the Dining Philosopher problem with (Sys V) semaphores
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>

//color macros
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\033[0;36m"

//amount of philosophers participating
#define N 5
//amount of time philosophers should eat
#define EAT 100

struct philosopher {
	int timeEating;
	int timeThinking;
};	

//prototypes
int randomGaussian(int mean, int stddev);

int main(int agrc, char **argv) {
  int semID;
  pid_t procID;
  int leftChop, rightChop;
  int timeEat, timeThink;

  //creating philosopher structures
  struct philosopher philosopher[N];
  //initializing starting values for each philosopher
  for(int i=0; i<N; i++) {
  	philosopher[i].timeEating=0;
  	philosopher[i].timeThinking=0;
  }

  //initializing a set of semephores for putting down chopsticks
  struct sembuf putDownChopstick[N] = {
  	{0, 1, 0},
  	{1, 1, 0},
  	{2, 1, 0},
  	{3, 1, 0},
  	{4, 1, 0}
  };

  //initialzing a set of semephores for picking up chopsticks
  struct sembuf pickUpChopstick[N] = {
    {0, -1, 0},
    {1, -1, 0},
    {2, -1, 0}, 
    {3, -1, 0},
    {4, -1, 0}
  };
  
  //creating a set of semephores and saving the ID
  semID=semget(IPC_PRIVATE, N, IPC_CREAT | IPC_EXCL | 0600);
  if(semID == -1) {
    fprintf(stdout, "%s\n", strerror(errno));
    exit(1);
  }
  //setting value of each semephore to 1 initially
  if(semop(semID, putDownChopstick, N) != 0) {
    fprintf(stdout, "%s\n", strerror(errno));
    exit(1);
  }
  //looping though to create N amount of child processes
  for(int i=0; i<N; i++) {
  	procID=fork();
    //failed to create child process
    if(procID < 0) {
      fprintf(stdout, "%s\n", strerror(errno));
      exit(1);
    }
    //child process
    if(procID == 0) {
      //getting left chopstick position
      leftChop = (i+(N-1))%N;
      //getting right chopstick position
      rightChop = i%N;
      //seeding random time
      srand(time(NULL)-2*i/2);
      
      while(1) {
        //checking if the philosophers left and right chopsticks are available 
        if(semctl(semID, leftChop, GETVAL) == 1 && semctl(semID, rightChop, GETVAL) == 1) {
          //picking up left and right chopsticks
          if(semop(semID, &pickUpChopstick[leftChop], 1) != 0) {
            fprintf(stdout, "%s\n", strerror(errno));
            exit(1);
          }
          if(semop(semID, &pickUpChopstick[rightChop], 1) != 0) {
            fprintf(stdout, "%s\n", strerror(errno));
            exit(1);
          }  
          //getting a time to eat
          timeEat=randomGaussian(9, 3);
          //if time to eat is negative set it to 0
          if(timeEat < 0) timeEat=0;
          printf(KGRN "Philosopher %i Is Eating For %i Seconds!\n", i, timeEat);
          //add the eat time to current philosopher
          philosopher[i].timeEating+=timeEat;
          //sleep for the eat time
          sleep(timeEat);
          //put down left and right chopsticks
          if(semop(semID, &putDownChopstick[leftChop], 1) != 0) {
            fprintf(stdout, "%s\n", strerror(errno));
            exit(1);
          }
          if(semop(semID, &putDownChopstick[rightChop], 1) != 0) {
            fprintf(stdout, "%s\n", strerror(errno));
            exit(1);  
          }
          //getting time to think
          timeThink=randomGaussian(11, 7);
          //if time to think is negative set it to 0
          if(timeThink < 0) timeThink=0;
          printf(KCYN "Philosopher %i Is Thinking For %i Seconds!\n", i, timeThink);
          //add the think time to current philosopher
          philosopher[i].timeThinking+=timeThink;
          //sleep for the think time
          sleep(timeThink);
        }
        //exit when current philosopher 
        if(philosopher[i].timeEating >= EAT) {
          printf(KRED "Philosopher %i Finished! They Ate for %i Seconds And Thought For %i Seconds!\n", i, philosopher[i].timeEating, philosopher[i].timeThinking);
          exit(0);
        }
      }
    }
  }
  //parent process
  for(int i=0; i<N; i++) {
    //wait on child processes to finish
    if(procID != 0) {
      wait(NULL);
    } 
  }
  //cleaning up all used semaphores
  for(int i = 0; i < N; i++) {
    semctl(semID, i, IPC_RMID, 0);

  } 
  return(0);
}

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */
int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}
