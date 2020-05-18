# Name: Jivan Gubbi
# ID: 604929653
# Email: jivangubbi@gmail.com



default:
	gcc -Wall -Wextra -g -o simpsh lab1c.c
clean:
	rm -fr simpsh *.txt *.o *.tar.gz
dist:
	tar -czvf lab1-604929653.tar.gz lab1c.c README Makefile performance.pdf
check: default flags chdir abort ignore wait
	if [ -s error.txt ]; then \
		cat error.txt; \
	else \
		echo "Passed Smokecheck"; \
		fi;
flags:
	echo "testing" > in.txt; \
	./simpsh --rdonly in.txt --creat --rdwr out.txt --creat --wronly err.txt --command 0 1 2 echo "hello"; \
	if [ $$? == 0 ] && [ -e out.txt ] && [ -e err.txt ] && grep -q "hello" out.txt; then \
		echo "Passed flag test"; \
	else \
		echo "Failed flag test" > error.txt; \
	fi;
chdir:
	rm -fr testFolder; \
	mkdir testFolder; \
	./simpsh --rdonly in.txt --wronly err.txt --chdir testFolder --creat --rdwr out1.txt --command 0 2 1 echo "hello"; \
	if [ $$? == 0 ] && [ -e testFolder/out1.txt ] && [ -e err.txt ] && grep -q "hello" testFolder/out1.txt; then \
		echo "Passed chdir test"; \
	else \
		echo "Failed chdir test" > error.txt; \
	fi;
abort:
	./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --abort 12 --command 0 1 2 echo "hello"; \
	if [ $$? == 139 ]; then \
		echo "Passed abort test"; \
	else \
		echo "Failed abort test" > error.txt; \
	fi;
ignore:
	./simpsh --verbose --catch 1 --ignore 1 --abort 1 --rdonly a.txt > output.txt; \
	if [ -s output.txt ]; then \
		echo "Passed ignore"; \
	else \
		echo "Failed ignore" > error.txt; \
	fi;
wait:
	./simpsh --verbose --rdwr in.txt --command 0 0 0 sleep 2 --command 0 0 0 sleep 3 --wait --command 0 0 0 sleep 4 --wait > test1.txt 2>err1.txt; \
	if [ ! -s err1.txt ] && grep -q "sleep 3" test1.txt && grep -q "sleep 4" test1.txt; then \
		echo "Passed wait"; \
	else \
		echo "Failed wait" > error.txt; \
	fi;
