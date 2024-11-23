CC = gcc
CFLAGS = -Wall -Wextra -Werror -pipe -ggdb -std=c99

OBJS = hash_map.o mem.o tokenizer.o skvs.o arraylist.o

# valgrind --leak-check=full --show-leak-kinds=all -s ./tfidf

.PHONY: default
default:
	@exit 1

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

tfidf: $(OBJS) main.c
	$(CC) $(CFLAGS) $(OBJS) -o tfidf main.c

.PHONY: build
build: tfidf

test_map: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o test_map test_map.c
	./test_map

.PHONY: clean
clean:
	@rm $(OBJS) tfidf
