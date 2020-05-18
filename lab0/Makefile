# Name: Jivan Gubbi
# ID: 604929653
# Email: jivangubbi@gmail.com



default:
	gcc -Wall -Wextra -g -o lab0 lab0.c
clean:
	rm -fr lab0 *.txt *.o *.tar.gz
dist:
	tar -czvf lab0-604929653.tar.gz lab0.c README backtrace.png \
	Makefile breakpoint.png
check: | default input output inout segfault catch dumpcore argfail basic
	if [ -s error.txt ]; then \
		cat error.txt; \
	else \
		echo "Passed Smokecheck"; \
		fi;
input: 
	./lab0 --input=nosuchfile.txt > /dev/null 2>&1;\
	if [ $$?==2 ]; then \
		echo "Passed bad input file"; \
	else \
		echo "Failed bad input file" > error.txt; \
	fi;
output:
	touch badoutput.txt; \
	chmod u-w badoutput.txt; \
	echo "hello" | ./lab0 --output=badoutput.txt > /dev/null 2>&1; \
	if [ $$?==3 ]; then \
		echo "Passed bad output file"; \
	else \
		echo "Failed bad output file" >> error.txt; \
	fi;
inout:
	touch inout.txt; \
	echo "testing" > inout.txt; \
	./lab0 --input=inout.txt --output=comp.txt; \
	if cmp -s inout.txt comp.txt; then \
		echo "Passed in and out file test"; \
	else \
		echo "Failed in and out file test" >> error.txt; \
	fi;
segfault:
	 ./lab0 --segfault > /dev/null 2>&1; \
        if [ $$?==139 ]; then \
                echo "Passed segfault"; \
        else \
                echo "Failed segfault" > error.txt; \
        fi;
catch:
	./lab0 --segfault --catch > /dev/null 2>&1; \
        if [ $$?==4 ]; then \
                echo "Passed catch"; \
        else \
                echo "Failed catch" > error.txt; \
        fi;

dumpcore:
	./lab0 --segfault --catch --dumpcore > /dev/null 2>&1; \
        if [ $$?==139 ]; then \
                echo "Passed dumpcore"; \
        else \
                echo "Failed dumpcore" > error.txt; \
        fi;

argfail:
	./lab0 --weirdarg > /dev/null 2>&1; \
        if [ $$?==1 ]; then \
                echo "Passed bad argument"; \
        else \
                echo "Failed bad argument" > error.txt; \
        fi;

basic:
	touch test2.txt; \
	echo "testing" > test2.txt; \
	cat test2.txt | ./lab0 > comp.txt; \
        if cmp -s test2.txt comp.txt; then \
		echo "Passed in and out file test"; \
        else \
		echo "Failed in and out file test" >> error.txt; \
        fi;
