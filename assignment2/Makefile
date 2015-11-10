#Mac frameworks
LDFLAGS =
#Compiler flags, enable warnings
GCCFLAGS = -Wall -lpthread
#SOURCES that are needed for all sub-compilations
SOURCES = main.c clock.c display.c si_comm.c si_ui.c

NAME = alarm_clock

all :  $(NAME)

$(NAME) : $(SOURCES)
	g++ $(GCCFLAGS) $(LDFLAGS) $(SOURCES) -o $(NAME)

clean :
	rm -f $(NAME)
	rm -f *.o
