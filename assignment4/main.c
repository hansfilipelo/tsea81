#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "lift.h"
#include "si_ui.h"
#include "messages.h"
#include <string.h>
#include "draw.h"

#define QUEUE_UI 0
#define QUEUE_LIFT 1
#define QUEUE_FIRSTPERSON 10
#define QUEUE_FILE 2

#define _MAX_ITERATIONS_ 5

// These variables keeps track of the process IDs of all processes
// involved in the application so that they can be killed when the
// exit command is received.
static pid_t lift_pid;
static pid_t uidraw_pid;
static pid_t liftmove_pid;
static pid_t person_pid[MAX_N_PERSONS];
static pid_t file_pid;

static FILE *output_file;



typedef enum {LIFT_TRAVEL, // A travel message is sent to the list process when a person would
  // like to make a lift travel
  LIFT_TRAVEL_DONE, // A travel done message is sent to a person process when a
  // lift travel is finished
  LIFT_MOVE,         // A move message is sent to the lift task when the lift shall move
  // to the next floor

  REQUEST_TO_WRITE, // passenger request to write to file
  OK_TO_WRITE, // file process sends ok to passenger
  FINISHED_WRITING // person has written to file

} lift_msg_type;

struct lift_msg{
  lift_msg_type type;  // Type of message
  int person_id;       // Specifies the person
  int from_floor;      // Specify source and destion for the LIFT_TRAVEL message.
  int to_floor;
};



// Since we use processes now and not
static int get_random_value(int person_id, int maximum_value)
{
  return rand() % (maximum_value + 1);
}


// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
  srand(getpid()); // The pid should be a good enough initialization for
  // this case at least.
}


static void liftmove_process(void)
{
  struct lift_msg m;
  m.type = LIFT_MOVE;

  while(1){
    //    Send a message to the lift process to move the lift.
    message_send((char *) &m, sizeof(m), QUEUE_LIFT, 0);
  }
}

static void lift_process(void)
{
  lift_type Lift;
  Lift = lift_create();
  int change_direction, next_floor;

  char msgbuf[4096];
  while(1){
    int i;
    int raknare = 0;
    int it = 0;
    struct lift_msg reply;
    struct lift_msg *m;
    int len = message_receive(msgbuf, 4096, QUEUE_LIFT); // Wait for a message
    if(len < sizeof(struct lift_msg)){
      fprintf(stderr, "Message too short\n");
      continue;
    }

    m = (struct lift_msg *) msgbuf;
    switch(m->type){
      case LIFT_MOVE:

      //    Check if passengers want to leave elevator
      for(i = 0; i < MAX_N_PASSENGERS; i++)
      {
        if(Lift->passengers_in_lift[i].to_floor == Lift->floor)
        {
          //        Send a LIFT_TRAVEL_DONE for each passenger that leaves
          //        the elevator
          reply.type = LIFT_TRAVEL_DONE;
          reply.person_id = Lift->passengers_in_lift[i].id;
          message_send((char *) &reply, sizeof(reply), QUEUE_FIRSTPERSON + reply.person_id, 0);

          //        Remove the passenger from the elevator
          Lift->passengers_in_lift[i].id = NO_ID;
          Lift->passengers_in_lift[i].to_floor = NO_FLOOR;
        }
      }

      for(i = 0; i < MAX_N_PERSONS; i++)
      {
        //    Check if passengers want to enter elevator
        if(Lift->persons_to_enter[Lift->floor][i].id != NO_ID && n_passengers_in_lift(Lift) < MAX_N_PASSENGERS)
        {
          //        Remove the passenger from the floor and into the elevator
          int j;
          for(j = 0; j < MAX_N_PASSENGERS; j++)
          {
            if (Lift->passengers_in_lift[j].id == NO_ID) {
              Lift->passengers_in_lift[j].id = Lift->persons_to_enter[Lift->floor][i].id;
              Lift->passengers_in_lift[j].to_floor = Lift->persons_to_enter[Lift->floor][i].to_floor;
              break;
            }
          }

          Lift->persons_to_enter[Lift->floor][i].id = NO_ID;
          Lift->persons_to_enter[Lift->floor][i].to_floor = NO_FLOOR;

        }
      }

      //    Move the lift
      lift_next_floor(Lift, &next_floor, &change_direction);
      lift_move(Lift, next_floor, change_direction);
      change_direction = 0;

      break;
      case LIFT_TRAVEL:

      // --------- Print passengers in lift and pass to enter

      // for (raknare = 0; raknare < N_FLOORS; raknare++) {
      //   for (it = 0; it < MAX_N_PASSENGERS; it++) {
      //     printf("%i ",Lift->persons_to_enter[raknare][it].id);
      //   }
      //   printf("\n");
      // }

      // --------------------

      //    Update the Lift structure so that the person with the given ID  is now present on the floor
      for(i = 0; i < MAX_N_PERSONS; i++)
      {
        if (Lift->persons_to_enter[m->from_floor][i].id == NO_ID) {
          Lift->persons_to_enter[m->from_floor][i].id = m->person_id;
          Lift->persons_to_enter[m->from_floor][i].to_floor = m->to_floor;
          break;
        }
      }
      break;
    }
  }
  return;
}

static void person_process(int id)
{
  init_random();
  char buf[4096];
  struct lift_msg *m_recieve;
  struct lift_msg m_send;

  struct timeval starttime;
  struct timeval endtime;

  long long int* timediffs[_MAX_ITERATIONS_];
  int counter = 0;

  while(1){
    if ( counter < _MAX_ITERATIONS_) {

      //    Generate a to and from floor
      //    Send a LIFT_TRAVEL message to the lift process
      m_send.type = LIFT_TRAVEL;
      m_send.person_id = id;
      m_send.from_floor = get_random_value(id, N_FLOORS - 1);
      m_send.to_floor = get_random_value(id, N_FLOORS - 1);
      message_send((char *) &m_send, sizeof(m_send), QUEUE_LIFT, 0);

      gettimeofday(&starttime, NULL);
      //    Wait for a LIFT_TRAVEL_DONE message
      int len = message_receive(buf, 4096, QUEUE_FIRSTPERSON + id); // Wait for a message

      gettimeofday(&endtime, NULL);

      timediffs[counter] = (endtime.tv_sec*1000000ULL + endtime.tv_usec) - (starttime.tv_sec*1000000ULL + starttime.tv_usec);
      counter++;
    }
    else {

      int i;
      char write_string[40*_MAX_ITERATIONS_];
      char line[40];

      // send request to write
      m_send.type = REQUEST_TO_WRITE;
      m_send.person_id = id;
      message_send((char *) &m_send, sizeof(m_send), QUEUE_FILE, 1);


      message_receive(buf, 4096, QUEUE_FIRSTPERSON + id);
      m_recieve = (struct lift_msg *) buf;
      while(m_recieve->type != OK_TO_WRITE)
      {
        // wait for ok to write
        printf("Not OK to write yet: ");
        printf("%d\n",m_recieve->type);
        message_receive(buf, 4096, QUEUE_FIRSTPERSON + id);
        m_recieve = (struct lift_msg *) buf;
      }
      printf("Person %i allowed to write \n", id);

      output_file = fopen("stats.txt", "a");
      if (output_file == NULL)
      {
        printf("Error opening file!\n");
        exit(1);
      }

      for (i = 0; i < _MAX_ITERATIONS_; i++) {
        sprintf(line,"%i",timediffs[i]);
        strcat(line,"\n");
        strcat(write_string,line);
        memset(line, 0,sizeof(line[0])*40);
      }
      fputs(write_string,output_file);
      fclose(output_file);

      // message finished writing to file
      m_send.type = FINISHED_WRITING;
      m_send.person_id = id;
      message_send((char *) &m_send, sizeof(m_send), QUEUE_FILE, 1);
      printf("Person %i finished writing \n", id);

      return;
    }


  }
}

// This is the final process called by main()
// It is responsible for:
//   * Receiving and executing commands from the java GUI
//   * Killing off all processes when exiting the application
void uicommand_process(void)
{
  int i;
  int person_counter = 0;
  char message[SI_UI_MAX_MESSAGE_SIZE];

  for (i = 0; i < MAX_N_PERSONS; i++) {
    person_pid[person_counter] = fork();
    if (!person_pid[person_counter]) {
      person_process(person_counter);
    }
    person_counter++;
  }

  return;

}

void file_process(void)
{
  char buf[4096];
  struct lift_msg *m_recieve;
  struct lift_msg m_send;

  int persons_to_write = 0;
  int persons_done;

  // wait for message from all persons
  while(persons_to_write < MAX_N_PERSONS)
  {
    message_receive(buf, 4096, QUEUE_FILE);
    m_recieve = (struct lift_msg *) buf;

    if(m_recieve->type != REQUEST_TO_WRITE)
    {
      printf("Not a request to write message \n");
    }
    persons_to_write++;
  }

  // send ok to write and wait for finished response to each person
  for (persons_done = 0; persons_done < MAX_N_PERSONS; persons_done++) {
    m_send.type = OK_TO_WRITE;
    m_send.person_id = persons_done;
    message_send((char *) &m_send, sizeof(m_send), QUEUE_FIRSTPERSON + persons_done, 1);

    printf("\nSent allowing message to person %i \n", persons_done);

    message_receive(buf, 4096, QUEUE_FILE);
    m_recieve = (struct lift_msg *) buf;

    while(m_recieve->type != FINISHED_WRITING)
    {
      printf("Not a finished writing message. Message of type: ");
      printf("%d\n",m_recieve->type);
      printf("Message from %i \n", m_recieve->person_id);
      message_receive(buf, 4096, QUEUE_FILE);
      m_recieve = (struct lift_msg *) buf;
    }
    printf("Received a finished writing message from %i\n", persons_done);

  }

  printf("Should kill \n");
  int i;
  kill(uidraw_pid, SIGINT);
  kill(lift_pid, SIGINT);
  kill(liftmove_pid, SIGINT);
  for(i=0; i < MAX_N_PERSONS; i++){
    kill(person_pid[i], SIGINT);
  }
  exit(0);

}

// This process is responsible for drawing the lift. Receives lift_type structures
// as messages.
void uidraw_process(void)
{
  char msg[1024];
  si_ui_set_size(670, 700);
  while(1){
    message_receive(msg, 1024, QUEUE_UI);
    lift_type Lift = (lift_type) &msg[0];
    draw_lift(Lift);
  }
}

int main(int argc, char **argv)
{
  message_init();

  lift_pid = fork();
  if(!lift_pid) {
    lift_process();
  }

  uicommand_process();

  uidraw_pid = fork();
  if(!uidraw_pid){
    uidraw_process();
  }
  liftmove_pid = fork();
  if(!liftmove_pid){
    liftmove_process();
  }
  file_pid = fork();
  if(!file_pid){
    file_process();
  }

  return 0;
}
