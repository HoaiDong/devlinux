all:
	gcc -Wall -Wextra -pthread -Iinclude ./src/main.c -o ./test/server
debug:
	gcc -Wall -Wextra -pthread -DDEBUG -Iinclude -g ./src/main.c -o ./test/server
run:
	./test/server 10 15
clean:
	rm -rf ./test/server
