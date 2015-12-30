#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <mqueue.h>
#include <time.h>
#include <errno.h> /* errno and perror */
#include <sys/wait.h> // Waitpid

#define QUEUE_START 1000
#define MAX_MESSAGES_IN_QUEUE 10
#define NR_OF_SAMPLES 128


static pid_t sampler;
static pid_t worker;

// Queue properties
int perms = 0600; /* permissions */
int write_flags = O_CREAT | O_WRONLY;
int read_flags = O_CREAT | O_RDONLY;
int rd=0, wr=0; /* -r and -w options */
char* name = "/samples";

struct mq_attr buffer; /* buffer for stat info */

//-------

int get_sample()
{
  return rand();
}

//-------

void sample_process()
{
  mqd_t sample_queue;
  sample_queue = mq_open(name, write_flags, perms, &buffer);
  if (sample_queue == (mqd_t)-1) {
    printf("Failed to open queue. \n Error: %s\n", strerror(errno));
    mq_close(sample_queue);
    mq_unlink(name);
    exit(1);
  }

  char message[NR_OF_SAMPLES];

  while (1) {
    for (size_t i = 0; i < NR_OF_SAMPLES; i++) {
      usleep(500);
      //printf("%i\n", i);
      message[i] = (char)i;
    }

    if(mq_send(sample_queue, message, NR_OF_SAMPLES, 0) == -1)
    {
      printf("Failed to send message! \n");
      printf("Error: %s\n", strerror(errno));
    }
  }
}

//-------

void fft(char* indata)
{
  int i;
  for(i = 0; i < NR_OF_SAMPLES; i++)
  {
    printf("%i\n", indata[i]);
  }
}

//-------

void worker_process()
{
  mqd_t sample_queue;
  sample_queue = mq_open(name, read_flags, perms,&buffer);
  if (sample_queue == (mqd_t)-1) {
    printf("Failed to open queue. \n Error: %s\n", strerror(errno));
    mq_close(sample_queue);
    mq_unlink(name);
    exit(1);
  }

  char message[NR_OF_SAMPLES];

  while (1) {
    if (mq_receive(sample_queue, message, NR_OF_SAMPLES, 0) == -1) {
      printf("Failed to receive message! \n");
      printf("Error: %s\n", strerror(errno));
    }

    fft(message);

  }
}

//-------

int main()
{
  // Initialize variables
  buffer.mq_msgsize = NR_OF_SAMPLES;
  buffer.mq_maxmsg = MAX_MESSAGES_IN_QUEUE;
  srand(time(NULL));

  // Create processes
  sampler = fork();
  if (!sampler) {
    sample_process();
  }

  worker = fork();
  if (!worker) {
    worker_process();
  }

  int return_status;
  waitpid(worker, &return_status, 0);
}
