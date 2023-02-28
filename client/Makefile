TARGET=client
CC=g++
SRCS=client.cpp main.cpp window.cpp client_command.cpp
OBJS=client.o main.o window.o client_command.o
LIB=-lSDL2 -lSDL2_image -lm -lSDL2_ttf -lSDL2_gfx -lSDL2_mixer

all: $(TARGET)

$(TARGET): $(SRCS) $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB)

.c.o: $(SRCS)
	$(CC) -c $(SRCS)

clean:
	rm -rf *.a *.o *~ $(TARGET)