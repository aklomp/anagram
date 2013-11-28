CFLAGS += -std=c89 -O3 -Wall -Wextra -Werror -pedantic

.PHONY: clean all

OBJS := anagram.o histogram.o

all: anagram test/test

anagram: $(OBJS)
	$(CC) $(LDFLAGS) -o "$@" $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o "$@" -c $^

test/test: histogram.o test/test.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) anagram test/test test/test.o
