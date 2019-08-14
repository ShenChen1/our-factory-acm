all:
	gcc hex.c -o hex
	gcc -g -O3 -o main.o -Wall -Werror -ffunction-sections -fdata-sections -c main.c
	gcc -Wl,--gc-sections main.o -lm -o main
	g++ -g main.c -o main.bak -Wall -Werror
	strip -s main
	./hex main > tmp.c
	cat template.c >> tmp.c
	g++ tmp.c

