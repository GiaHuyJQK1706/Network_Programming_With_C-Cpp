CC = g++
CFLAGS = -Wall -pthread -std=c++17
TARGET_SERVER = server
TARGET_CLIENT = client

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): server.cpp
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) server.cpp

$(TARGET_CLIENT): client.cpp
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) client.cpp

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)