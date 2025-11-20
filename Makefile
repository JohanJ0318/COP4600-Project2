CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
TARGET = chash
OBJS = chash.o hash_operations.o hash_utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

chash.o: chash.c chash.h
	$(CC) $(CFLAGS) -c chash.c

hash_operations.o: hash_operations.c chash.h
	$(CC) $(CFLAGS) -c hash_operations.c

hash_utils.o: hash_utils.c chash.h
	$(CC) $(CFLAGS) -c hash_utils.c

clean:
	rm -f $(TARGET) $(OBJS) hash.log

.PHONY: all clean
