
all:
	g++ -g -Wall -Werror test1.cpp -o test1
	g++ -g -Wall -Werror test2.cpp -o test2
	g++ -g -Wall -Werror test3.cpp -o test3
	g++ -g -Wall -Werror test4.cpp -o test4

clean:
	rm -rf test1 test2 test3 test4

