#Mac frameworks
LDFLAGS =
COMPILER = gcc
#Compiler flags, enable warnings
GCCFLAGS = -g3 -Wall -pthread -lpthread
#SOURCES that are needed for all sub-compilations
SOURCES = clock.c display.c si_comm.c si_ui.c

NAME = alarm_clock

all :  $(NAME)

$(NAME) : $(SOURCES)
	$(COMPILER) $(GCCFLAGS) $(LDFLAGS) $(SOURCES) -o $(NAME)

clean :
	rm -f $(NAME)
	rm -f *.o
