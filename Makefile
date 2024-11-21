CC = gcc
CFLAGS = -Wall -Wextra -Werror -pipe -ggdb -std=c99

OBJS = map.o mem.o

.PHONY: default
default:
	@exit 1

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: build
build: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o tfidf main.c

.PHONY: test_map
test_map: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o test_map test_map.c
	./test_map
