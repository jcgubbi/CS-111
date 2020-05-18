default:
	gcc -Wall -Wextra -g -lm -lmraa lab4b.c -o lab4b
clean:
	rm -fr lab4b *.o *.tar.gz
dist:
	tar -czf lab4b-604929653.tar.gz lab4b.c README Makefile
check: default
	echo "done"

