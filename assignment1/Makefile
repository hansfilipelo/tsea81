all: set_clock odd_even 

set_clock: set_clock.c si_comm.c si_ui.c si_ui.h si_comm.h
	gcc -lpthread set_clock.c si_comm.c si_ui.c -g  -o set_clock -Wall

odd_even: odd_even.c si_comm.c si_comm.h si_ui.c si_ui.h
	gcc -lpthread odd_even.c si_comm.c si_ui.c -g  -o odd_even -Wall





clean:
	rm -f odd_even set_clock *~ *.o
