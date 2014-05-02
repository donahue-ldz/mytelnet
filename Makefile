CC = gcc
CFLAGS = -Wall -O -g3
LFLAGS = -lcurses
TARGET = mytelnet
SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

#%.o : %.c
#	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(INCLUDE) $(LFLAGS)

.PHONY:clean
clean:
	$(RM) $(TARGET) $(OBJS)  *.bak
