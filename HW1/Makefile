CC = gcc
CFLAGS = -Wall -g
TARGET = hw1

FILESRC = main.c list.c

FILEO = $(FILESRC:.c=.o)

all: $(TARGET)

$(TARGET): $(FILEO)
	$(CC) $(CFLAGS) -o $(TARGET) $(FILEO)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(FILEO) $(TARGET)
