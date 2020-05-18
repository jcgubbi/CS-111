#!/usr/bin/gnuplot
set terminal png
set datafile separator ","

set title "Throughput vs. Number of Threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput(ops/s)"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'spin lock' with linespoints lc rgb 'green'

set title "Wait Time and Time per Operation vs Threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Time/Operation(ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Time per Op' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Wait Time' with linespoints lc rgb 'green'

set title "Threads vs Successful Iterations"
set xlabel "Threads"
set logscale x 2
set ylabel "Iteration"
set logscale y 10
set output 'lab2b_3.png'

plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red', \
     "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'green', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Spin Lock' with points lc rgb 'blue'


set title "Threads vs. Throughput for Mutex"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput(ops/s)"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '16 lists' with linespoints lc rgb 'purple'

set title "Threads vs. Throughput for Spin Lock"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput(ops/s)"
set logscale y 10
set output 'lab2b_5.png'
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '16 lists' with linespoints lc rgb 'purple'
