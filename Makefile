CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Isimulador -Itables
TARGET  = simulator

SRCS = main.c \
       simulador/simulator.c \
       tables/dense.c \
       tables/twoLevel.c \
       tables/threeLevel.c \
       tables/inverted.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean