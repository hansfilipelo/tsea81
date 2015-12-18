#ifndef QUEUE_H
#define QUEUE_H
#include <sys/time.h>
#include <stdio.h>

/* maximum number of persons in the lift system */
#define MAX_N_PERSONS 10
#define NR_OF_JOURNEYS 100

/* fig_begin person_data_type */
/* data structure for person information */
typedef struct
{
  /* identity */
  int id;
  /* destination floor */
  int to_floor[NR_OF_JOURNEYS];
  int from_floor[NR_OF_JOURNEYS];

  struct timeval starttime;
  struct timeval endtime;

  long long int timediffs[NR_OF_JOURNEYS];

  // Counter which tells us which "journey" we are traveling
  int journey;

} person_data_type;
/* fig_end person_data_type */

typedef struct
{
  person_data_type passengers[MAX_N_PERSONS+1];
  int head;
  int tail;

} queue;

void queue_init(queue* queue_in);
int empty_queue(queue* queue_in);
void add_to_queue(queue* queue_in, person_data_type* person_in);
person_data_type pop_from_queue(queue* queue_in);

#endif
