#include "lift.h"

/* Simple_OS include */
#include <pthread.h>

/* drawing module */
#include "draw.h"

/* standard includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* --- monitor data type for lift and operations for create and delete START --- */

/* lift_create: creates and initialises a variable of type lift_type */
lift_type lift_create(void)
{
    /* the lift to be initialised */
    lift_type lift;

    /* floor counter */
    int floor;

    /* loop counter */
    int i;

    /* allocate memory */
    lift = (lift_type) malloc(sizeof(lift_data_type));

    /* initialise variables */

    /* initialise floor */
    lift->floor = 0;

    /* set direction of lift travel to up */
    lift->up = 1;

    /* the lift is not moving */
    lift->moving = 0;

    for(i = 0; i < N_FLOORS; i++)
    {
      queue_init(&lift->persons_to_enter[i]);
    }

    /* initialise passenger information */
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        lift->passengers_in_lift[i] = NO_PERSON;
    }

    // Initiate an empty person
    NO_PERSON.id = NO_ID;
    for(i = 0; i < NR_OF_JOURNEYS; i++)
    {
      NO_PERSON.to_floor[i] = NO_FLOOR;
      NO_PERSON.from_floor[i] = NO_FLOOR;
      NO_PERSON.timediffs[i] = 0;
    }

    return lift;
}


/* --- monitor data type for lift and operations for create and delete END --- */


/* --- functions related to lift task START --- */

/* MONITOR function lift_next_floor: computes the floor to which the lift
   shall travel. The parameter *change_direction indicates if the direction
   shall be changed */
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction)
{
  if (lift->up == 0) {
    if (lift->floor == 1) {
      *change_direction = 1;
    }
    *next_floor = lift->floor - 1;
  }
  else if (lift->up == 1) {
    if (lift->floor == N_FLOORS-2) {
      *change_direction = 1;
    }
    *next_floor = lift->floor + 1;
  }
}

void lift_move(lift_type lift, int next_floor, int change_direction)
{

    /* the lift has arrived at next_floor */
    lift->floor = next_floor;

    /* check if direction shall be changed */
    if (change_direction)
    {
        lift->up = !lift->up;
    }

}

/* this function is used also by the person tasks */
int n_passengers_in_lift(lift_type lift)
{
    int n_passengers = 0;
    int i;

    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        if (lift->passengers_in_lift[i].id != NO_ID)
        {
            n_passengers++;
        }
    }
    return n_passengers;
}

/* --- functions related to lift task END --- */
