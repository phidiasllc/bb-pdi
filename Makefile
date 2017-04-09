all: program

program.o: program.cc gpio.hh Makefile
	g++ -Wall -Wextra -ggdb3 -O3 $< -c -o $@

program: program.o Makefile
	g++ -Wall -Wextra -ggdb3 -O3 $< -o $@

clean:
	rm *.o

.PHONY: clean
