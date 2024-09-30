CC = gcc
CFLAGS = -lcurl
TARGET = resolver

all: $(TARGET)

$(TARGET): main.c
	$(CC) main.c -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET) *.csv
