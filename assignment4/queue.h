
#include "lift.h"

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
