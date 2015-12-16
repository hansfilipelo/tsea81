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
//#include "draw.h"
#include <sys/wait.h>

#define QUEUE_LIFT 1
#define QUEUE_FIRSTPERSON 10
#define QUEUE_FILE 2

#define MAX_ITERATIONS 10000

// These variables keeps track of the process IDs of all processes
// involved in the application so that they can be killed when the
// exit command is received.
static pid_t lift_pid;
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
  int i;
  int change_direction, next_floor;
  person_data_type temp_person;

  char msgbuf[4096];
  while(1){
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

      for(i = 0; i < MAX_N_PASSENGERS; i++)
      {
        //    Check if passengers want to enter elevator
        if(!empty_queue(&Lift->persons_to_enter[Lift->floor]) && n_passengers_in_lift(Lift) < MAX_N_PASSENGERS )
        {

          temp_person = pop_from_queue(&Lift->persons_to_enter[Lift->floor]);
          //        Remove the passenger from the floor and into the elevator
          int j;
          for(j = 0; j < MAX_N_PASSENGERS; j++)
          {
            if (Lift->passengers_in_lift[j].id == NO_ID) {
              Lift->passengers_in_lift[j].id = temp_person.id;
              Lift->passengers_in_lift[j].to_floor = temp_person.to_floor;
              break;
            }
          }
        }
      }

      //    Move the lift
      lift_next_floor(Lift, &next_floor, &change_direction);
      lift_move(Lift, next_floor, change_direction);
      change_direction = 0;

      break;
      case LIFT_TRAVEL:
      //    Update the Lift structure so that the person with the given ID  is now present on the floor
      temp_person.id = m->person_id;
      temp_person.to_floor = m->to_floor;
      add_to_queue(&Lift->persons_to_enter[m->from_floor], &temp_person);
      break;

      case LIFT_TRAVEL_DONE:
      printf("Undefined state!\n");
      break;
      case REQUEST_TO_WRITE:
      printf("Undefined state!\n");
      break;
      case OK_TO_WRITE:
      printf("Undefined state!\n");
      break;
      case FINISHED_WRITING:
      printf("Undefined state!\n");
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

  long long int timediffs[MAX_ITERATIONS];
  int counter = 0;

  while(1){
    if ( counter < MAX_ITERATIONS) {

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
      if(len < sizeof(struct lift_msg)){
        fprintf(stderr, "Message too short\n");
        continue;
      }

      gettimeofday(&endtime, NULL);

      timediffs[counter] = (endtime.tv_sec*1000000ULL + endtime.tv_usec) - (starttime.tv_sec*1000000ULL + starttime.tv_usec);
      counter++;
    }
    else {

      int i;
      char write_string[40*MAX_ITERATIONS];
      char line[40];

      // send request to write
      m_send.type = REQUEST_TO_WRITE;
      m_send.person_id = id;
      message_send((char *) &m_send, sizeof(m_send), QUEUE_FILE, 1);


      // wait for ok to write ------------
      message_receive(buf, 4096, QUEUE_FIRSTPERSON + id);
      m_recieve = (struct lift_msg *) buf;
      while(m_recieve->type != OK_TO_WRITE)
      {
        message_receive(buf, 4096, QUEUE_FIRSTPERSON + id);
        m_recieve = (struct lift_msg *) buf;
      }
      // ----------------------------------
      // Assemble data and write

      char filename[15];
      sprintf(filename, "stat_%d", MAX_N_PERSONS);
      strcat(filename,".txt");
      output_file = fopen(filename, "a");


      if (output_file == NULL)
      {
        printf("Error opening file!\n");
        exit(1);
      }

      // Concat one string so that we write only once to file (one disk access)
      for (i = 0; i < MAX_ITERATIONS; i++) {
        sprintf(line,"%lli",timediffs[i]);
        strcat(line,"\n");
        strcat(write_string,line);
        memset(line, 0,sizeof(line[0])*40);
      }
      // Write
      fputs(write_string,output_file);
      fclose(output_file);

      // ---------------------------------

      // message finished writing to file thread
      m_send.type = FINISHED_WRITING;
      m_send.person_id = id;
      message_send((char *) &m_send, sizeof(m_send), QUEUE_FILE, 1);

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

  // --------------------------
  // wait for REQUEST_TO_WRITE message from all persons
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

  // Kill lift processes since there's no passengers travelling
  kill(lift_pid, SIGINT);
  kill(liftmove_pid, SIGINT);

  // --------------------------
  // send ok to write and wait for finished response to each person
  for (persons_done = 0; persons_done < MAX_N_PERSONS; persons_done++) {
    // Assemble message
    m_send.type = OK_TO_WRITE;
    m_send.person_id = persons_done;
    message_send((char *) &m_send, sizeof(m_send), QUEUE_FIRSTPERSON + persons_done, 1);

    // Wait here for a FINISHED_WRITING-message
    message_receive(buf, 4096, QUEUE_FILE);
    m_recieve = (struct lift_msg *) buf;

    while(m_recieve->type != FINISHED_WRITING)
    {
      message_receive(buf, 4096, QUEUE_FILE);
      m_recieve = (struct lift_msg *) buf;
    }

    printf("Person %i finished writing \n", persons_done);
  }

  // Kill all remaining processes and cleanup
  int i;
  for(i=0; i < MAX_N_PERSONS; i++){
    kill(person_pid[i], SIGINT);
  }

  exit(0);

}


int main(int argc, char **argv)
{
  message_init();

  lift_pid = fork();
  if(!lift_pid) {
    lift_process();
  }

  liftmove_pid = fork();
  if(!liftmove_pid){
    liftmove_process();
  }

  file_pid = fork();
  if(!file_pid){
    file_process();
  }

  uicommand_process();

  // Wait for filepid to exit
  int returnStatus;
  waitpid(file_pid, &returnStatus, 0);

  return 0;
}
