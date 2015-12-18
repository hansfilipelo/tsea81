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

#define MAX_ITERATIONS 10000 // Need to be a multiple of NR_OF_JOURNEYS

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
  int from_floor[NR_OF_JOURNEYS];      // Specify source and destion for the LIFT_TRAVEL message.
  int to_floor[NR_OF_JOURNEYS];
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
  int j;
  int change_direction, next_floor;
  person_data_type temp_person;
  printf("Size of message: %lu\n", sizeof(struct lift_msg));

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
        if(Lift->passengers_in_lift[i].to_floor[Lift->passengers_in_lift[i].journey] == Lift->floor)
        {
          // Save RTT of journey
          gettimeofday(&Lift->passengers_in_lift[i].endtime, NULL);
          Lift->passengers_in_lift[i].timediffs[Lift->passengers_in_lift[i].journey] = (Lift->passengers_in_lift[i].endtime.tv_sec*1000000ULL + Lift->passengers_in_lift[i].endtime.tv_usec) - (Lift->passengers_in_lift[i].starttime.tv_sec*1000000ULL + Lift->passengers_in_lift[i].starttime.tv_usec);

          if(Lift->passengers_in_lift[i].journey == NR_OF_JOURNEYS - 1)
          {
            //        Send a LIFT_TRAVEL_DONE for each passenger that leaves elevator and has
            //        finished all journeys.
            reply.type = LIFT_TRAVEL_DONE;
            reply.person_id = Lift->passengers_in_lift[i].id;

            for(j = 0; j < NR_OF_JOURNEYS; j++)
            {
              reply.to_floor[j] = (int)Lift->passengers_in_lift[i].timediffs[j];
            }
            message_send((char *) &reply, sizeof(reply), QUEUE_FIRSTPERSON + reply.person_id, 0);
          }
          else
          {
            // Else continue with next journey
            temp_person = Lift->passengers_in_lift[i];
            temp_person.journey++;
            gettimeofday(&temp_person.starttime, NULL);
            add_to_queue(&Lift->persons_to_enter[temp_person.from_floor[temp_person.journey]], &temp_person);
          }

          //        Remove the passenger from the elevator
          Lift->passengers_in_lift[i] = NO_PERSON;
        }
      }

      for(i = 0; i < MAX_N_PASSENGERS; i++)
      {
        //    Check if passengers want to enter elevator
        if( !empty_queue(&Lift->persons_to_enter[Lift->floor]) && n_passengers_in_lift(Lift) < MAX_N_PASSENGERS )
        {

          temp_person = pop_from_queue(&Lift->persons_to_enter[Lift->floor]);
          //        Remove the passenger from the floor and into the elevator
          int j;
          for(j = 0; j < MAX_N_PASSENGERS; j++)
          {
            if (Lift->passengers_in_lift[j].id == NO_ID) {
              Lift->passengers_in_lift[j] = temp_person;
              break;
            }
          }
        }
        else{
          break;
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

      for(i = 0; i < NR_OF_JOURNEYS; i++)
      {
        temp_person.to_floor[i] = m->to_floor[i];
        temp_person.from_floor[i] = m->from_floor[i];
      }
      temp_person.journey = 0;
      gettimeofday(&temp_person.starttime, NULL);
      add_to_queue(&Lift->persons_to_enter[m->from_floor[0]], &temp_person);
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

  long long int timediffs[MAX_ITERATIONS];
  int counter = 0;

  while(1){
    if ( counter < MAX_ITERATIONS ) {

      //    Generate a to and from floor
      //    Send a LIFT_TRAVEL message to the lift process
      m_send.type = LIFT_TRAVEL;
      m_send.person_id = id;
      int i;
      for(i = 0; i < NR_OF_JOURNEYS; i++)
      {
        m_send.from_floor[i] = get_random_value(id, N_FLOORS - 1);
        m_send.to_floor[i] = get_random_value(id, N_FLOORS - 1);
        while(m_send.from_floor[i] == m_send.to_floor[i])
        {
          m_send.to_floor[i] = get_random_value(id, N_FLOORS - 1);
        }
      }

      message_send((char *) &m_send, sizeof(m_send), QUEUE_LIFT, 0);


      //    Wait for a LIFT_TRAVEL_DONE message
      int len = message_receive(buf, 4096, QUEUE_FIRSTPERSON + id); // Wait for a message
      if(len < sizeof(struct lift_msg)){
        fprintf(stderr, "Message too short\n");
        continue;
      }
      m_recieve = (struct lift_msg *) buf;

      for(i = 0; i < NR_OF_JOURNEYS; i++)
      {
        timediffs[counter++] = m_recieve->to_floor[i]; // Message size optimization
            // Using to_floor array to send back timediffs
      }
    }
    else {

      int i;
      char write_string[40*MAX_ITERATIONS];
      char line[40];

      // ----------------------------------
      // Assemble data and write

      char filename[16];
      sprintf(filename, "stat_%d_%d", MAX_N_PERSONS, id);
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
      printf("Person with id %i finished writing. \n", id);
      
	while(1);
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

  uicommand_process();

  // Wait for filepid to exit
  int returnStatus;
  waitpid(lift_pid, &returnStatus, 0);

  return 0;
}
