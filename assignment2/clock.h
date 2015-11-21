#ifndef CLOCK_H
#define CLOCK_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include "display.h"
#include <stdbool.h>
#include <time.h>


/* clock_init: initialise clock */
void clock_init(void);

/* clock_set_time: set current time to hours, minutes and seconds */
void clock_set_time(int hours, int minutes, int seconds);

/* get_time: read time from common clock variables */
void clock_get_time(int *hours, int *minutes, int *seconds);

#endif
