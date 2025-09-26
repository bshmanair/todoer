.SILENT:

all:
	g++ -g -O0 -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized -Wconversion -fsanitize=address,undefined task-cli.cpp -o task-cli

run:
	./task-cli

clean:
	rm ./task-cli ./test