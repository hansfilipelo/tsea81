#ifndef CLOCK_H
#define CLOCK_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>

/* clock_init: initialise clock */
void clock_init(void);

/* clock_set_time: set current time to hours, minutes and seconds */
void clock_set_time(int hours, int minutes, int seconds);

#endif
