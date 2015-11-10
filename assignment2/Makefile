#Mac frameworks
LDFLAGS =
#Compiler flags, enable warnings
GCCFLAGS = -Wall -lpthread
#SOURCES that are needed for all sub-compilations
SOURCES = clock.c display.c set_clock.c si_comm.c si_ui.c

NAME = alarm_clock

all :  $(NAME)

$(NAME) : $(SOURCES)
	g++ $(GCCFLAGS) $(LDFLAGS) $(SOURCES) -o $NAME

clean :
	rm -f $(NAME)
	rm -f *.o
