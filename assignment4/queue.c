#include "queue.h"

int empty_queue(queue* queue_in)
{
  return queue_in->head == queue_in->tail;
}

// ----------------

void add_to_queue(queue* queue_in, person_data_type* person_in)
{
  queue_in->passengers[queue_in->tail] = *person_in;
  if(queue_in->tail == MAX_N_PERSONS)
  {
    queue_in->tail = 0;
  }
  else
  {
    queue_in->tail++;
  }
}

// ----------------

person_data_type pop_from_queue(queue* queue_in)
{
  person_data_type temp_person;
  temp_person = queue_in->passengers[queue_in->head];
  if(queue_in->head == MAX_N_PERSONS)
  {
    queue_in->head = 0;
  }
  else
  {
    queue_in->head++;
  }
  return temp_person;
}

// ---------------

void queue_init(queue* queue_in)
{
  queue_in->head = 0;
  queue_in->tail = 0;
}
