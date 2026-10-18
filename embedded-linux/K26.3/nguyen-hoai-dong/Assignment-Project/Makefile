all: compile link
	gcc -Wall -Wextra -pthread -Iinclude ./src/sensor_node.c -o ./test/sensor_node
compile:
	gcc -Wall -Wextra -pthread -Iinclude -c ./src/main.c -o ./test/main.o
	gcc -Wall -Wextra -pthread -Iinclude -c ./src/log_process.c -o ./test/log_process.o
link:
	gcc -Wall -Wextra -pthread -Iinclude ./test/*.o -o ./test/server

debug: compile_debug link_debug
	gcc -Wall -Wextra -pthread -Iinclude -DDEBUG ./src/sensor_node.c -o ./test/sensor_node

compile_debug:
	gcc -Wall -Wextra -pthread -Iinclude -c -DDEBUG ./src/main.c -o ./test/main.o
	gcc -Wall -Wextra -pthread -Iinclude -c -DDEBUG ./src/log_process.c -o ./test/log_process.o
link_debug:
	gcc -Wall -Wextra -pthread -Iinclude ./test/*.o -o ./test/server

run:
	./test/server 1234 15
clean:
	rm -rf ./test/server ./test/*.o
