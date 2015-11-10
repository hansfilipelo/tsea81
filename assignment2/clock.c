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

// ------
// Get alarm time

void clock_get_alarm_time(int* hours, int* minutes, int* seconds){

  pthread_mutex_lock(&Clock.mutex);

  *hours = Clock.alarm_time.hours;
  *minutes = Clock.alarm_time.minutes;
  *seconds = Clock.alarm_time.seconds;

  pthread_mutex_unlock(&Clock.mutex);

}

// ----------
/* clock_task: clock task */

void *clock_thread(void *unused)
{

  /* time for next update */
  struct timespec ts;
  /* initialise time for next update */
  clock_gettime(CLOCK_MONOTONIC, &ts);

  /* local copies of the current time */
  int hours, minutes, seconds;
  int alarmHours, alarmMinutes, alarmSeconds;

  /* infinite loop */
  while (1)
  {
    /* read and display current time */
    clock_get_time(&hours, &minutes, &seconds);
    clock_get_alarm_time(&alarmHours,&alarmMinutes,&alarmSeconds);
    display_time(hours, minutes, seconds);

    /* increment time */
    clock_increment_time();

    if (alarmHours == hours && alarmMinutes == minutes && alarmSeconds == seconds && Clock.alarm_enabled == 1) {
      sem_post(&Clock.start_alarm);
    }

    /* compute time for next update */
    ts.tv_nsec += delay;
    if(ts.tv_nsec >= 1000*1000*1000){
      ts.tv_nsec -= 1000*1000*1000;
      ts.tv_sec++;
    }
    /* wait until time for next update */
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
  }
}

// -----------

void clock_set_alarm_time(int hours, int minutes, int seconds){

  pthread_mutex_lock(&Clock.mutex);

  Clock.alarm_time.hours = hours;
  Clock.alarm_time.minutes = minutes;
  Clock.alarm_time.seconds = seconds;
  Clock.alarm_enabled = 1;

  pthread_mutex_unlock(&Clock.mutex);

  display_alarm_time(hours, minutes, seconds);

}

// ------
// Gets alarm status

int clock_get_alarm_status(){

  pthread_mutex_lock(&Clock.mutex);

  if (Clock.alarm_enabled == 0){
    pthread_mutex_unlock(&Clock.mutex);
    return 0;
  }
  else if(Clock.alarm_enabled == 1){
    pthread_mutex_unlock(&Clock.mutex);
    return 1;
  }

  si_ui_show_error("Alarm enable in undefined state.");
  Clock.alarm_enabled = 0;
  pthread_mutex_unlock(&Clock.mutex);
  return 0;
}

// ----------
// Alarm task

void *clock_alarm_thread(void *unused)
{

  while (1) {
    sem_wait(&Clock.start_alarm);

    while (clock_get_alarm_status() == 1) {
      /* time for next update */
      struct timespec ts;
      /* initialise time for next update */
      clock_gettime(CLOCK_MONOTONIC, &ts);

      display_alarm_text();

      /* compute time for next update */
      ts.tv_nsec += delay;
      if(ts.tv_nsec >= 1500*1000*1000){
        ts.tv_nsec -= 1500*1000*1000;
        ts.tv_sec++;
      }
      /* wait until time for next update */
      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
    }
  }
}

// ---------
// Resets Alarm

void clock_reset_alarm(){
  pthread_mutex_lock(&Clock.mutex);
  Clock.alarm_enabled = 0;
  pthread_mutex_unlock(&Clock.mutex);

  erase_alarm_time();
  erase_alarm_text();
  sem_post(&Clock.start_alarm);
}

// ---------
// Helper functions

/* time_from_set_message: extract time from set message from user interface */
void time_from_set_message(char message[], int *hours, int *minutes, int *seconds)
{
  sscanf(message,"set %d %d %d",hours, minutes, seconds);
}

/* */
void time_from_alarm_message(char message[], int *hours, int *minutes, int *seconds)
{
  sscanf(message,"alarm %d %d %d",hours, minutes, seconds);
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


void * read_from_gui_thread(void *unused)
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
    /* check if it is an alarm set message */
    else if (strncmp(message, "alarm", 5) == 0)
    {
      time_from_alarm_message(message, &hours, &minutes, &seconds);
      if (time_ok(hours, minutes, seconds))
      {
        clock_set_alarm_time(hours, minutes, seconds);
      }
      else
      {
        si_ui_show_error("Illegal value for hours, minutes or seconds");
      }
    }
    else if (strcmp(message, "reset") == 0){
      clock_reset_alarm();
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
  pthread_t read_from_gui_thread_handle;
  pthread_t clock_alarm_thread_handle;

  pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
  pthread_create(&read_from_gui_thread_handle, NULL, read_from_gui_thread, 0);
  pthread_create(&clock_alarm_thread_handle, NULL, clock_alarm_thread, 0);

  pthread_join(clock_thread_handle, NULL);
  pthread_join(read_from_gui_thread_handle, NULL);
  pthread_join(clock_alarm_thread_handle, NULL);
  /* will never be here! */
  return 0;
}
