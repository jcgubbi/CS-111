# Name: Jivan Gubbi
# ID: 604929653
# Email: jivangubbi@gmail.com



default:
	gcc -Wall -Wextra -g -o simpsh lab1a.c
clean:
	rm -fr simpsh *.txt *.o *.tar.gz
dist:
	tar -czvf lab1-604929653.tar.gz lab1a.c README Makefile
check: default rdonly wronly verbose fulltest
	if [ -s error.txt ]; then \
		cat error.txt; \
	else \
		echo "Passed Smokecheck"; \
		fi;
rdonly:
	echo "testing" > in.txt; \
	touch out.txt; \
	touch err.txt; \
	chmod err.txt 777; \
	chmod out.txt 777; \
	./simpsh --rdonly a.txt --wronly out.txt --wronly --err.txt; \
	if [ $$? == 1 ]; then \
		echo "Passed bad rdonly"; \
	else \
		echo "Failed bad rdonly" > error.txt; \
	fi;
wronly:
	./simpsh --rdonly in.txt --wronly rand.txt --wronly --err.txt; \
	if [ $$? == 1 ]; then \
		echo "Passed bad wronly"; \
	else \
		echo "Failed bad wronly" > error.txt; \
	fi;
fulltest:
	./simpsh --rdonly in.txt --wronly out.txt --wronly --err.txt --command 0 1 2 cat >newfile.txt; \
	if [ $$? == 0 ] && [ -s err.txt ] && [ ! -s out.txt ]; then \
		echo "Passed full test"; \
	else \
		echo "Failed full test"; \
	fi;
verbose:
	./simpsh --verbose --rdonly a.txt > output.txt; \
	if [ -s output.txt ]; then \
		echo "Passed verbose"; \
	else \
		echo "Failed verbose"; \
	fi;
