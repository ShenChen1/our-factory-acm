all:
	gcc hex.c -o hex
	g++ -g -O3 main.c -o main -Wall -Werror
	g++ -g main.c -o main.bak -Wall -Werror
	strip -s main
	./hex main > tmp.c
	cat template.c >> tmp.c
	g++ tmp.c



