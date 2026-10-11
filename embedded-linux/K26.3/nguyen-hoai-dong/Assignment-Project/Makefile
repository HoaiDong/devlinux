all:
	gcc -Wall -Wextra -pthread ./src/main.c -o ./test/server
debug:
	gcc -Wall -Wextra -pthread -DDEBUG ./src/main.c -o ./test/server
run:
	./test/server
clean:
	rm -rf ./test/server