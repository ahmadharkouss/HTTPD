CC = gcc
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -g
all: httpd

httpd: src/main.o src/parsing.o src/utils_server.o src/server.o src/daemon.o src/my_strcasecmp.o src/syntaxerr.o src/log.o
	$(CC) $(CFLAGS) -o httpd src/main.o src/parsing.o src/utils_server.o src/server.o src/daemon.o src/my_strcasecmp.o src/syntaxerr.o src/log.o

check: httpd
	./tests/test.sh

leak1: src/main.o src/parsing.o src/utils_server.o src/server.o src/daemon.o src/my_strcasecmp.o src/syntaxerr.o src/log.o
	$(CC) $(CFLAGS) -o httpd src/main.o src/parsing.o src/utils_server.o src/server.o src/daemon.o src/my_strcasecmp.o src/syntaxerr.o src/log.o -fsanitize=address -static-libasan

leaks: httpd
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./server1

clean:
	rm -rf httpd src/*.o leak1