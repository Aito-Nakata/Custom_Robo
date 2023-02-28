TARGET=server
CC=g++
SRCS=server.cpp main.cpp server_command.cpp
OBJS=server.o main.o server_command.o
LIB=-lSDL2 -lSDL2_image -lm -lSDL2_ttf -lSDL2_gfx

all: $(TARGET)

$(TARGET): $(SRCS) $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB)

.cpp.o: $(SRCS)
	$(CC) -c $(SRCS)

clean:
	rm -rf *.a *.o *~ $(TARGET)