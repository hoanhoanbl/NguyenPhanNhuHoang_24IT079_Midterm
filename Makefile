# Makefile - build the ls clone

CC      = cc
CFLAGS  = -Wall -Wextra -g
TARGET  = myls

# Modules compiled so far (more are added in later phases).
SRCS    = main.c options.c util.c
HDRS    = options.h util.h
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

.PHONY: all clean
