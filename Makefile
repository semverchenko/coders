CFLAGS=-g -Wall -Wextra -pedantic
LDFLAGS=-g

PROGRAM = main

all: $(PROGRAM)

%.o: %.c
	$(CC) -c  $(CFLAGS) -MD $<

include $(wildcard *.d)

$(PROGRAM): $(patsubst %.c,%.o,$(wildcard *.c))
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o *.d $(PROGRAM) *.core
