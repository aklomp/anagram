CFLAGS += -std=c89 -O3 -Wall -Wextra -Werror -pedantic

.PHONY: clean test

OBJS := anagram.o histogram.o

anagram: $(OBJS)
	$(CC) $(LDFLAGS) -o "$@" $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o "$@" -c $^

test/test: histogram.o test/test.o
	$(CC) $(LDFLAGS) -o $@ $^

test: test/test
	./test/test

clean:
	rm -f $(OBJS) anagram test/test test/test.o
