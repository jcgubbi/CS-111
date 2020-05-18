# Name: Jivan Gubbi
# ID: 604929653
# Email: jivangubbi@gmail.com



default:
	gcc -Wall -Wextra -pthread -g -o lab2_list SortedList.c lab2_list.c
	gcc -Wall -Wextra -pthread -g -o lab2_add lab2_add.c
clean:
	rm -fr lab2_add lab2_list  *.txt *.o *.tar.gz
dist: default graphs
	tar -czf lab2a-604929653.tar.gz README Makefile lab2_add.c lab2_list.c SortedList.c SortedList.h lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png graph.sh part1.gp part2.gp
graphs: tests
	gnuplot part1.gp
	gnuplot part2.gp
tests:
	./graph.sh
