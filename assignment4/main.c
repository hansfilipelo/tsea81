#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include "lift.h"
#include "si_ui.h"
#include "messages.h"

#include "draw.h"

#define QUEUE_UI 0
#define QUEUE_LIFT 1
#define QUEUE_FIRSTPERSON 10

// These variables keeps track of the process IDs of all processes
// involved in the application so that they can be killed when the
// exit command is received.
static pid_t lift_pid;
static pid_t uidraw_pid;
static pid_t liftmove_pid;
static pid_t person_pid[MAX_N_PERSONS];



typedef enum {LIFT_TRAVEL, // A travel message is sent to the list process when a person would
    // like to make a lift travel
    LIFT_TRAVEL_DONE, // A travel done message is sent to a person process when a
    // lift travel is finished
    LIFT_MOVE         // A move message is sent to the lift task when the lift shall move
    // to the next floor
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
		//    Sleep 2 seconds
        sleep(2);
        //    Send a message to the lift process to move the lift.
        message_send((char *) &m, sizeof(m), QUEUE_LIFT, 0);
	}
}

static void lift_process(void)
{
<<<<<<< Updated upstream
    lift_type Lift;
	Lift = lift_create();
	int change_direction, next_floor;
	
	char msgbuf[4096];
	while(1){
		int i;
		struct lift_msg reply;
		struct lift_msg *m;
		message_send((char *) Lift, sizeof(*Lift), QUEUE_UI,0); // Draw the lift
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
=======
  lift_type Lift;
  Lift = lift_create();
  int i;
  int j;
  int change_direction, next_floor;
  person_data_type temp_person;
  printf("%lu\n", sizeof(struct lift_msg));

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
      Lift->floor++;
      //    Check if passengers want to leave elevator
      for(i = 0; i < MAX_N_PASSENGERS; i++)
      {
        //printf("Lift->passengers_in_lift[i].to_floor[0]: %i \n", Lift->passengers_in_lift[i].to_floor[0]);
        if(Lift->passengers_in_lift[i].to_floor[Lift->passengers_in_lift[i].journey] == Lift->floor)
        {
          printf("Found people to leave lift. \n");
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
            printf("Sending TRAVEL_DONE\n");
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

      //printf("Not empty lift: %i \n", empty_queue(&Lift->persons_to_enter[Lift->floor]) == 1);
        //printf("Persons in lift: %i \n", n_passengers_in_lift(Lift));
        //printf("ID of first in lift: %i \n", Lift->passengers_in_lift[0].id);
        //    Check if passengers want to enter elevator
        if( empty_queue(&Lift->persons_to_enter[Lift->floor]) == 0 && n_passengers_in_lift(Lift) < MAX_N_PASSENGERS )
        {
          printf("Found people to enter lift. \n");

          temp_person = pop_from_queue(&Lift->persons_to_enter[Lift->floor]);
          //        Remove the passenger from the floor and into the elevator

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
      printf("RECEIVE LIFT_TRAVEL. \n");
      //    Update the Lift structure so that the person with the given ID  is now present on the floor
      temp_person.id = m->person_id;

      for(i = 0; i < NR_OF_JOURNEYS; i++)
      {
        temp_person.to_floor[i] = m->to_floor[i];
        temp_person.from_floor[i] = m->from_floor[i];
      }
      temp_person.journey = 0;
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
>>>>>>> Stashed changes
}

static void person_process(int id)
{
	init_random();
	char buf[4096];
	struct lift_msg m;
	while(1){
        //    Generate a to and from floor
		//    Send a LIFT_TRAVEL message to the lift process
        m.type = LIFT_TRAVEL;
        m.person_id = id;
        m.from_floor = get_random_value(id, N_FLOORS - 1);
        m.to_floor = get_random_value(id, N_FLOORS - 1);
        message_send((char *) &m, sizeof(m), QUEUE_LIFT, 0);
        
        //    Wait for a LIFT_TRAVEL_DONE message
        int len = message_receive(buf, 4096, QUEUE_FIRSTPERSON + id); // Wait for a message
        if(len < sizeof(struct lift_msg)){
			fprintf(stderr, "Message too short\n");
			continue;
		}
        m = *(struct lift_msg *) buf;
        // This should never happen:
        if (m.type != LIFT_TRAVEL_DONE) {
            fprintf(stderr, "Not a LIFT_TRAVEL_DONE message\n");
        }
<<<<<<< Updated upstream
        
		//    Wait a little while
        sleep(5);
	}
=======
      }
      printf("Sending LIFT_TRAVEL\n");
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
>>>>>>> Stashed changes
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
    
	while(1){
		// Read a message from the GUI
		si_ui_receive(message);
        // * Check that we don't create too many persons
		if(!strcmp(message, "new")){
            if(person_counter > MAX_N_PERSONS - 1) {
                si_ui_show_error("No more passengers allowed");
            }
            // * fork and create a new person process (and
			//   record the new pid in person_pid[])
            else {
                person_pid[person_counter] = fork();
                if (!person_pid[person_counter]) {
                    person_process(person_counter);
                }
                person_counter++;
            }
		}else if(!strcmp(message, "exit")){
			// The code below sends the SIGINT signal to
			// all processes involved in this application
			// except for the uicommand process itself
			// (which is exited by calling exit())
			kill(uidraw_pid, SIGINT);
			kill(lift_pid, SIGINT);
			kill(liftmove_pid, SIGINT);
			for(i=0; i < MAX_N_PERSONS; i++){
				if(person_pid[i] > 0){
					kill(person_pid[i], SIGINT);
				}
			}
			exit(0);
		}
	}
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
    si_ui_init(); // Initialize user interface. (Must be done
    // here!)
    
	lift_pid = fork();
	if(!lift_pid) {
		lift_process();
	}
	uidraw_pid = fork();
	if(!uidraw_pid){
		uidraw_process();
	}
	liftmove_pid = fork();
	if(!liftmove_pid){
		liftmove_process();
	}
	uicommand_process();
    
	return 0;
}
