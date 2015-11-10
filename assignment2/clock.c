#include "clock.h"

// ---------
/* time data type */

typedef struct
{
    int hours;
    int minutes;
    int seconds;
} time_data_type;

// ---------
/* clock data type */

typedef struct
{
    /* the current time */
    time_data_type time;
    /* alarm time */
    time_data_type alarm_time;
    /* alarm enabled flag */
    int alarm_enabled;

    /* semaphore for mutual exclusion */
    pthread_mutex_t mutex;

    /* semaphore for alarm activation */
    sem_t start_alarm;

} clock_data_type;

// ---------
/* the actual clock */

static clock_data_type Clock;

// ---------
/* clock_init: initialise clock */

void clock_init(void)
{
    /* initialise time and alarm time */

    Clock.time.hours = 0;
    Clock.time.minutes = 0;
    Clock.time.seconds = 0;

    Clock.alarm_time.hours = 0;
    Clock.alarm_time.minutes = 0;
    Clock.alarm_time.seconds = 0;

    /* alarm is not enabled */
    Clock.alarm_enabled = 0;

    /* initialise semaphores */
    pthread_mutex_init(&Clock.mutex, NULL);
    sem_init(&Clock.start_alarm, 0, 0);
}

// ---------
/* clock_set_time: set current time to hours, minutes and seconds */

void clock_set_time(int hours, int minutes, int seconds)
{
    pthread_mutex_lock(&Clock.mutex);

    Clock.time.hours = hours;
    Clock.time.minutes = minutes;
    Clock.time.seconds = seconds;

    pthread_mutex_unlock(&Clock.mutex);
}

// --------
/* increment_time: increments the current time with
   one second */

void clock_increment_time(void)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* increment time */
    Clock.time.seconds++;
    if (Clock.time.seconds > 59)
    {
        Clock.time.seconds = 0;
        Clock.time.minutes++;
        if (Clock.time.minutes > 59)
        {
            Clock.time.minutes = 0;
            Clock.time.hours++;
            if (Clock.time.hours > 23)
            {
                Clock.time.hours = 0;
            }
        }
    }

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

// ----------
/* get_time: read time from common clock variables */

void clock_get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* read values */
    *hours = Clock.time.hours;
    *minutes = Clock.time.minutes;
    *seconds = Clock.time.seconds;

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

// ----------
/* clock_task: clock task */

void *clock_thread(void *unused)
{
    /* local copies of the current time */
    int hours, minutes, seconds;

    /* infinite loop */
    while (1)
    {
        /* read and display current time */
        clock_get_time(&hours, &minutes, &seconds);
        display_time(hours, minutes, seconds);

        /* increment time */
        clock_increment_time();

        /* wait one second */
        usleep(1000000);
    }
}

// ------
// Get alarm time

void clock_get_alarm_time(int* hours, int* minutes, int* seconds){

  pthread_mutex_lock(&Clock.mutex);

  *hours = Clock.alarm_time.hours;
  *minutes = Clock.alarm_time.minutes;
  *seconds = Clock.alarm_time.seconds;

  pthread_mutex_unlock(&Clock.mutex);

}

// ------
// Gets alarm status

int clock_get_alarm_status(){

  pthread_mutex_lock(&Clock.mutex);

  if (Clock.alarm_enabled == 0){
    return 0;
  }
  else if(Clock.alarm_enabled == 1){
    return 1;
  }
  else{
      si_ui_show_error("Alarm enable in undefined state.");
  }
  pthread_mutex_unlock(&Clock.mutex);
}

// ----------
// Alarm task

void *clock_alarm_thread(void *unused)
{
    /* local copies of the current time */
    int hours, minutes, seconds;

    /* infinite loop */
    while (1)
    {
        /* read and display current time */
        clock_get_alarm_time(&hours, &minutes, &seconds);
        display_alarm_time(hours, minutes, seconds);

        /* increment time */
        clock_increment_time();

        /* wait one second */
        usleep(1000000);
    }
}

// ---------
// Helper functions

/* time_from_set_message: extract time from set message from user interface */
void time_from_set_message(char message[], int *hours, int *minutes, int *seconds)
{
  sscanf(message,"set %d %d %d",hours, minutes, seconds);
}
/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */
int time_ok(int hours, int minutes, int seconds)
{
  return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 &&
  seconds >= 0 && seconds <= 59;
}

// ---
/* set_task: reads messages from the user interface, and
   sets the clock, or exits the program */


void * clock_set_thread(void *unused)
{
    /* message array */
    char message[SI_UI_MAX_MESSAGE_SIZE];

    /* time read from user interface */
    int hours, minutes, seconds;

    /* set GUI size */
    si_ui_set_size(400, 200);

    while(1)
    {
        /* read a message */
        si_ui_receive(message);
        /* check if it is a set message */
        if (strncmp(message, "set", 3) == 0)
        {
            time_from_set_message(message, &hours, &minutes, &seconds);
            if (time_ok(hours, minutes, seconds))
            {
                clock_set_time(hours, minutes, seconds);
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds");
            }
        }
        /* check if it is an exit message */
        else if (strcmp(message, "exit") == 0)
        {
            exit(0);
        }
        /* not a legal message */
        else
        {
            si_ui_show_error("unexpected message type");
        }
    }
}

// ----------
/* main program */

int main(void)
{
    /* initialise UI channel */
    si_ui_init();

    /* initialise variables */
    clock_init();

    /* create tasks */
    pthread_t clock_thread_handle;
    pthread_t clock_set_thread_handle;

    pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
    pthread_create(&clock_set_thread_handle, NULL, clock_set_thread, 0);

    pthread_join(clock_thread_handle, NULL);
    pthread_join(clock_set_thread_handle, NULL);
    /* will never be here! */
    return 0;
}
