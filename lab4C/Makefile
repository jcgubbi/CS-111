default:
	gcc -Wall -Wextra -g -lm -lmraa lab4c.c -o lab4c_tcp
	gcc -Wall -Wextra -g -lm -lmraa -lssl -lcrypto -Wno-deprecated-declarations lab4c_tls.c -o lab4c_tls
clean:
	rm -fr lab4b *.o *.tar.gz
dist:
	tar -czf lab4c-604929653.tar.gz lab4c.c README Makefile lab4c_tls.c
check: default
	echo "done"

