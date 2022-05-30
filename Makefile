CFLAGS += -std=c89 -O3 -Wall -Wextra -Werror -pedantic

.PHONY: analyze clean test

PROG := anagram
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test/test: src/histogram.o test/test.o

test: test/test
	./test/test

analyze: clean
	scan-build --status-bugs $(MAKE)

clean:
	$(RM) $(OBJS) $(PROG) test/test test/test.o
