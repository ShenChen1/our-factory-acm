all:
	gcc hex.c -o hex
	gcc -g -O3 main.c -o main -Wall -Werror -lm
	gcc -g main.c -o main.bak -Wall -Werror -lm
	strip -s main
	./hex main > tmp.c
	cat template.c >> tmp.c
	g++ tmp.c



