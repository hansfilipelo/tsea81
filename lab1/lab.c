#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

// This file contains the definition of MAX_ITERATIONS, among other things.
#include "samples.h"

// Delay in nanoseconds (1 millisecond)
#define DELAY 1000000

// Number of samples that do_work() handles
#define PROCESSING_INTERVAL  256

// Could be a local variable, but you may need it as a static variable
// here when you modify this file according to the lab instructions.
static int sample_buffer[2][PROCESSING_INTERVAL];
static pthread_mutex_t buffer_lock[2];
static sem_t wait_for_sample;
static pthread_mutex_t done_lock;
static int done = 0;

// -----------------------------

void do_work(int *samples)
{
  int i;

  //  A busy loop. (In a real system this would do something
  //  more interesting such as an FFT or a particle filter,
  //  etc...)
  volatile int dummy; // A compiler warning is ok here
  for(i=0; i < 20000000;i++){
    dummy=i;
  }

  // Write out the samples.
  for(i=0; i < PROCESSING_INTERVAL; i++){
    write_sample(0,samples[i]);
  }
}

// -----------------------------

void *do_work_task(void *arg)
{
  // Set higher priority for this task
  struct sched_param sp;
  sp.sched_priority = 5;
  if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)){
		fprintf(stderr,"WARNING: Failed to set do_work_task to real-time priority\n");
	}

  int current_buffer = 0;

  while(1){
    pthread_mutex_lock(&done_lock);
    if (done == 1){
    	pthread_mutex_unlock(&done_lock);
        break;
    }
    pthread_mutex_unlock(&done_lock);

    sem_wait(&wait_for_sample);

    pthread_mutex_lock(&buffer_lock[current_buffer]);
    do_work(sample_buffer[current_buffer]);
    pthread_mutex_unlock(&buffer_lock[current_buffer]);

    current_buffer = !current_buffer;
  }
  return NULL;
}

// -----------------------------

struct timespec firsttime;
void *sample_task(void *arg)
{
  // Set higher priority for this task
  struct sched_param sp;
  sp.sched_priority = 10;
  if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)){
    fprintf(stderr,"WARNING: Failed to set sample_task to real-time priority\n");
  }

  int channel = 0;
  struct timespec current;
  int i;
  int samplectr = 0;
  current = firsttime;
  int current_buffer = 0;

  pthread_mutex_lock(&buffer_lock[current_buffer]);

  for(i=0; i < MAX_ITERATIONS; i++){
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);

    sample_buffer[current_buffer][samplectr] = read_sample(channel);
    samplectr++;

    if(samplectr == PROCESSING_INTERVAL){
      pthread_mutex_unlock(&buffer_lock[current_buffer]);
      samplectr = 0;
      current_buffer = !current_buffer;
      pthread_mutex_lock(&buffer_lock[current_buffer]);
      sem_post(&wait_for_sample);
    }

    // Increment current time point
    current.tv_nsec +=  DELAY;
    if(current.tv_nsec >= 1000000000){
      current.tv_nsec -= 1000000000;
      current.tv_sec++;
    }
  }

  pthread_mutex_lock(&done_lock);
  done = 1;
  pthread_mutex_unlock(&done_lock);
  return NULL;
}

// -----------------------------

int main(int argc,char **argv)
{
  pthread_t sample_thread;
  pthread_attr_t attr;
  pthread_t do_work_thread;

  // Init thread safe variables
  pthread_mutex_init(&buffer_lock[0],NULL);
  pthread_mutex_init(&buffer_lock[1],NULL);
  pthread_mutex_init(&done_lock,NULL);
  sem_init(&wait_for_sample, 0, 0);

  clock_gettime(CLOCK_MONOTONIC, &firsttime);

  // Start the sampling at an even multiple of a second (to make
  // the sample times easy to analyze by hand if necessary)
  firsttime.tv_sec+=2;
  firsttime.tv_nsec = 0;
  printf("Starting sampling at about t+2 seconds\n");

  samples_init(&firsttime);

  if(pthread_attr_init(&attr)){
    perror("pthread_attr_init");
  }
  // Set default stacksize to 64 KiB (should be plenty)
  if(pthread_attr_setstacksize(&attr, 65536)){
    perror("pthread_attr_setstacksize()");
  }

  // Lock this program in main memory - don't allow it to be swapped
  if(mlockall(MCL_FUTURE|MCL_CURRENT)){
    fprintf(stderr, "%s\n", "WARNING: Failed to lock memory!");
  }

  // Start worker threads
  pthread_create(&sample_thread, &attr, sample_task, NULL);
  pthread_create(&do_work_thread, &attr, do_work_task, NULL);

  pthread_join(sample_thread, NULL);
  pthread_join(do_work_thread, NULL);

  // Dump output data which will be used by the analyze.m script
  dump_outdata();
  dump_sample_times();
  printf("Data dumped.\n");
  return 0;
}
