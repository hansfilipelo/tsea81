#include "queue.h"
#include <stdio.h>

int main()
{
  queue test_queue;

  person_data_type test_person;
  queue_init(&test_queue);

  if(empty_queue(&test_queue))
  {
    printf("Queue is empty as it should be \n");
  }
  else {
    printf("Queue is not empty - but it should be \n");
  }

  printf("Adding persons to queue.\n");
  int i;
  for(i = 0; i < MAX_N_PERSONS; i++)
  {
    test_person.id = i;
    add_to_queue(&test_queue, &test_person);
    printf("Head: %i\n", test_queue.head);
    printf("Tail: %i\n", test_queue.tail);
    printf("\n");
  }

  if(empty_queue(&test_queue))
  {
    printf("Queue is empty - but it should not be.\n");
  }
  else {
    printf("Queue is not empty as it should be.\n");
  }

  for(i = 0; i < MAX_N_PERSONS; i++)
  {
    printf("Popping person with id: %i\n", pop_from_queue(&test_queue).id);
    printf("Head: %i\n", test_queue.head);
    printf("Tail: %i\n", test_queue.tail);
    printf("\n");
  }

  if(empty_queue(&test_queue))
  {
    printf("Queue is empty as it should be \n");
  }
  else {
    printf("Queue is not empty - but it should be \n");
  }
}
