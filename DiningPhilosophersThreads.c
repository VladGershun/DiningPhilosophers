/*
* Creator: Vlad Gershun
* Date: 10/26/2021
* Class: CS 360
* Project: HW 6 - Create a program to solve the Dining Philosopher problem with threading
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

//color macros
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\033[0;36m"

//number of philosophers participating
#define N 5
//amount of time philosophers should eat
#define EAT 100

//prototypes
int randomGaussian(int mean, int stddev);

struct Philosopher {
	int philID;
	int timeEating;
	int timeThinking;
	pthread_mutex_t *leftChop;
	pthread_mutex_t *rightChop;
};

pthread_mutex_t chopstick[N];

void* eat(void* arg) {
	int timeEat, timeThink;
	struct Philosopher* philosopher=(struct Philosopher*)arg;
  //seeding random time
  srand(time(NULL)-2*philosopher->philID/2);

  while(1) {
    //force lock left chopstick and try getting right chopstick
  	pthread_mutex_lock(philosopher->leftChop);
    if(pthread_mutex_trylock(philosopher->rightChop) == 0) {
      //getting a time to eat
      timeEat=randomGaussian(9, 3);
      //if time to eat is negative set it to 0
      if(timeEat < 0) timeEat=0;
      printf(KGRN "Philosopher %i Is Eating For %i Seconds!\n", philosopher->philID, timeEat);
      //add the eat time to current philosopher
      philosopher->timeEating+=timeEat;
      //sleep for the eat time
      sleep(timeEat);

    	//getting time to think
      timeThink=randomGaussian(11, 7);
      //if time to think is negative set it to 0
      if(timeThink < 0) timeThink=0;
      printf(KCYN "Philosopher %i Is Thinking For %i Seconds!\n", philosopher->philID, timeThink);
      //add the think time to current philosopher
      philosopher->timeThinking+=timeThink;

      //unlocking mutex corresponding to left and right chopsticks
      pthread_mutex_unlock(philosopher->leftChop);
      pthread_mutex_unlock(philosopher->rightChop);

      //sleep for the think time
      sleep(timeThink);

      //exit when current philosopher 
      if(philosopher->timeEating >= EAT) {
        printf(KRED "Philosopher %i Finished! They Ate for %i Seconds And Thought For %i Seconds!\n", philosopher->philID, philosopher->timeEating, philosopher->timeThinking);
        pthread_exit(NULL);
      }
    }
    //could not get right chopstick so put down left chopstick
    else {
      pthread_mutex_unlock(philosopher->leftChop);
    }
  }
	return(NULL);
}

int main(int argc, char** argv) {

  pthread_t p[N];
  int leftChop, rightChop;

  //initializing mutex for each chopstick
  for(int i=0; i<N; i++) {
  	pthread_mutex_init(&chopstick[i], NULL);
  }
	//creating philosopher structures
  struct Philosopher* philosopher[N];
  //initializing starting values for each philosopher
  for(int i=0; i<N; i++) {
  	leftChop=(i+(N-1))%N;
  	rightChop=i%N;
  	philosopher[i]=malloc(sizeof(struct Philosopher));
  	philosopher[i]->philID=i;
  	philosopher[i]->timeEating=0;
  	philosopher[i]->timeThinking=0;
  	philosopher[i]->leftChop=&chopstick[leftChop];
  	philosopher[i]->rightChop=&chopstick[rightChop];
  }
  //creating a thread for each philosopher
  for(int i=0; i<N; i++) {
    if(pthread_create(&p[i], NULL, eat, philosopher[i]) != 0) {
      fprintf(stdout, "%s\n", strerror(errno));
      return(1);
    }
  } 
  //joining all threads together
  for(int i=0; i<N; i++) {
    if(pthread_join(p[i], NULL) != 0) {
      fprintf(stdout, "%s\n", strerror(errno));
      return(1);
    }
  }
  //destroying all used mutexs
  for(int i=0; i<N; i++) {
    pthread_mutex_destroy(&chopstick[i]);
  }
  //freeing all created philosopher structures
  for(int i=0; i<N; i++) {
  	free(philosopher[i]);
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
