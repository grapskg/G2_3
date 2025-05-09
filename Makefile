CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -pthread

# Output executable
SERVER = server

# Object files
OBJS = server.o lists.o message.o utilities.o

# Only one header file
HEADERS = server.h

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER) $(OBJS) *~

.PHONY: all clean