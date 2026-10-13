# Makefile - build the ls clone

CC      = cc
CFLAGS  = -Wall -Wextra -g
TARGET  = myls

# Source modules and their headers.
SRCS    = main.c options.c util.c entry.c list.c sort.c print.c format.c
HDRS    = options.h util.h entry.h list.h sort.h print.h format.h
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rebuild objects when any header changes.
$(OBJS): $(HDRS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

# Compare the program with the system ls on a prepared directory tree.
test: all
	sh test.sh

.PHONY: all clean test
