
all:
	g++ -Wall -Werror test1.cpp -o test1
	g++ -Wall -Werror test2.cpp -o test2
	g++ -Wall -Werror test3.cpp -o test3

clean:
	rm -rf test1 test2 test3

