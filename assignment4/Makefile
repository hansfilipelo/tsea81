CFLAGS=-O1 -g -Wall
LIBS=-lrt -lpthread
CFILES=lift.c main.c si_ui.c si_comm.c messages.c queue.c
HEADERS=lift.h si_ui.h si_comm.h messages.h queue.h

QUEUECFILES=test_queue.c queue.c
QUEUEHFILES=queue.h

lift_messages: $(CFILES) $(HEADERS)
	gcc $(CFLAGS) $(CFILES) -o lift_messages $(LIBS) 

test_queue: $(QUEUECFILES) $(QUEUEHFILES)
	gcc $(CFLAGS) $(QUEUECFILES) -o test_queue $(LIBS)

clean:
	rm -f lift_messages *~ *.o test_queue
