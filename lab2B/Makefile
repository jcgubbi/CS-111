# Name: Jivan Gubbi
# ID: 604929653
# Email: jivangubbi@gmail.com



default:
	gcc -Wall -Wextra -pthread -g -o lab2_list SortedList.c lab2_list.c
clean:
	rm -fr lab2_list  *.txt *.o *.tar.gz
profile: default
	rm -f ./raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so.0 \
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_funct ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf
dist: default graphs
	tar -czf lab2b-604929653.tar.gz README Makefile lab2_list.c SortedList.c SortedList.h lab2b_list.csv lab2b_1.png graph.sh part2.gp lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png profile.out
graphs: tests
	gnuplot part2.gp
tests:
	./graph.sh
