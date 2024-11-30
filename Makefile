CC = gcc
CFLAGS = -Wall -Wextra -Werror -pipe -ggdb -std=c99
# CFLAGS = -Wall -Wextra -Werror -pipe -O3 -march=native -std=c99
CLINKFLAGS = -lm -lreadline

OBJS = hash_map.o tokenizer.o skvs.o arraylist.o arena.o

# valgrind --leak-check=full --show-leak-kinds=all -s ./tfidf

.PHONY: default
default:
	@exit 1

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(CLINKFLAGS)

tfidf: $(OBJS) main.c
	$(CC) $(CFLAGS) $(OBJS) -o tfidf main.c $(CLINKFLAGS)

.PHONY: build
build: tfidf

test: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o test test.c $(CLINKFLAGS)
	./test
	@valgrind --leak-check=full -s ./test
	@rm ./test

.PHONY: clean
clean:
	@rm $(OBJS) tfidf
