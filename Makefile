CFLAGS := -Wall -Wextra -Werror -std=c89 -pedantic

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
