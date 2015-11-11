#ifndef CLOCK_H
#define CLOCK_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include "display.h"
#include <stdbool.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <sys/time.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

/* clock_init: initialise clock */
void clock_init(void);

/* clock_set_time: set current time to hours, minutes and seconds */
void clock_time_set(int hours, int minutes, int seconds);

/* get_time: read time from common clock variables */
void clock_get_time(int *hours, int *minutes, int *seconds);

#endif
