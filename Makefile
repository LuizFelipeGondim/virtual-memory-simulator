CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = simulator
OBJS = main.o simulator.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c simulator.h
	$(CC) $(CFLAGS) -c main.c

simulator.o: simulator.c simulator.h
	$(CC) $(CFLAGS) -c simulator.c

clean:
	rm -f *.o $(TARGET)