#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "debug.h"
#include "lift.h"
#include "si_ui.h"
#include <string.h>
#include <sys/time.h>

#define _MAX_ITERATIONS_ 10000
FILE *output_file;
pthread_mutex_t file_mutex;
pthread_barrier_t thread_done_barrier;

// Counter which counts number of threads ready
int threads_ready = 0;
pthread_mutex_t counter_mutex;

// Unfortunately the rand() function is not thread-safe. However, the
// rand_r() function is thread-safe, but need a pointer to an int to
// store the current state of the pseudo-random generator.  This
// pointer needs to be unique for every thread that might call
// rand_r() simultaneously. The functions below are wrappers around
// rand_r() which should work in the environment encountered in
// assignment 3.
//

static unsigned int rand_r_state[MAX_N_PERSONS];
// Get a random value between 0 and maximum_value. The passenger_id
// parameter is used to ensure that the rand_r() function is called in
// a thread-safe manner.

static int get_random_value(int passenger_id, int maximum_value)
{
	return rand_r(&rand_r_state[passenger_id]) % (maximum_value + 1);
}

static lift_type Lift;
static sem_t id_wait; // Semaphore for waitin upon passenger creation

// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
	int i;
	for(i=0; i < MAX_N_PERSONS; i++){
		// Use this statement if the same random number sequence
		// shall be used for every execution of the program.
		rand_r_state[i] = i;

		// Use this statement if the random number sequence
		// shall differ between different runs of the
		// program. (As the current time (as measured in
		// seconds) is used as a seed.)
		rand_r_state[i] = i+time(NULL);
	}
}



static void *lift_thread(void *unused)
{
	int change_direction = 0;
	int next_floor;

	while(1){
		lift_next_floor(Lift, &next_floor, &change_direction);
		lift_move(Lift, next_floor, change_direction);
		lift_has_arrived(Lift);
		change_direction = 0;

		pthread_mutex_lock(&counter_mutex);
		if (threads_ready >= MAX_N_PERSONS) {
			break;
		}
		pthread_mutex_unlock(&counter_mutex);
	}
	exit(0);
}

static void *passenger_thread(void *idptr)
{
	// Code that reads the passenger ID from the idptr pointer
	// (due to the way pthread_create works we need to first cast
	// the void pointer to an int pointer).

	int *tmp = (int *) idptr;
	int id = *tmp;
	sem_post(&id_wait);

	struct timeval starttime;
	struct timeval endtime;

	long long int timediffs[_MAX_ITERATIONS_];
	int counter = 0;

	while(1){

		int from_floor = get_random_value(id, N_FLOORS - 1);
		int to_floor = get_random_value(id, N_FLOORS - 1);
		while(to_floor == from_floor) {
			from_floor = get_random_value(id, N_FLOORS - 1);
		}

		debug_check_override(id, &from_floor, &to_floor);

		gettimeofday(&starttime, NULL);

		lift_travel(Lift, id, from_floor, to_floor);

		gettimeofday(&endtime, NULL);

		if ( counter < _MAX_ITERATIONS_) {
			timediffs[counter] = (endtime.tv_sec*1000000ULL + endtime.tv_usec) - (starttime.tv_sec*1000000ULL + starttime.tv_usec);
			counter++;
		}
		else {

			pthread_barrier_wait(&thread_done_barrier);

			int i;
			char write_string[40*_MAX_ITERATIONS_];
			char line[40];
			pthread_mutex_lock(&file_mutex);

			char filename[15];
      sprintf(filename, "stat_%d", MAX_N_PERSONS);
      strcat(filename,".txt");
      output_file = fopen(filename, "a");

			if (output_file == NULL)
			{
					printf("Error opening file!\n");
					exit(1);
			}

			for (i = 0; i < _MAX_ITERATIONS_; i++) {
				sprintf(line,"%lli",timediffs[i]);
				strcat(line,"\n");
				strcat(write_string,line);
				memset(line, 0,sizeof(line[0])*40);
			}
			fputs(write_string,output_file);
			fclose(output_file);

			pthread_mutex_unlock(&file_mutex);

			// We're ready - give signal
			pthread_mutex_lock(&counter_mutex);
			threads_ready += 1;
			pthread_mutex_unlock(&counter_mutex);

			return 0;
		}
	}

	return NULL;
}

static void *user_thread(void *unused)
{
	int current_passenger_id = 0;
	sem_init(&id_wait, 0, 0);

	si_ui_set_size(670, 700);

	int i;
	for (i = 0; i < MAX_N_PERSONS; i++) {
		pthread_t passenger_thread_handle;

		pthread_create(&passenger_thread_handle, NULL, passenger_thread,&current_passenger_id);
		pthread_detach(passenger_thread_handle);

		sem_wait(&id_wait);
		current_passenger_id++;
	}
	return NULL;
}


int main(int argc, char **argv)
{
	// Mutex output file
	pthread_mutex_init(&file_mutex,NULL);
	pthread_barrier_init(&thread_done_barrier,NULL,MAX_N_PERSONS);
	pthread_mutex_init(&counter_mutex,NULL);
	// output stop

	debug_init();
	init_random();
	Lift = lift_create();

	pthread_t lift_thread_handle;
	pthread_t user_thread_handle;

	pthread_create(&lift_thread_handle, NULL, lift_thread,0);
	pthread_create(&user_thread_handle, NULL, user_thread,0);

	pthread_join(lift_thread_handle, NULL);
	pthread_join(user_thread_handle, NULL);


	return 0;
}
