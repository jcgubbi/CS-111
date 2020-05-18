#!/bin/bash

rm lab2b_list.csv

threads=(1 2 4 8 12 16 24)
iterations=(1000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_list --threads=$thr --iterations=$its --sync=m >> lab2b_list.csv
		./lab2_list --threads=$thr --iterations=$its --sync=s >> lab2b_list.csv
	done
done

threads=(1 4 8 12 16)
iterations=(1 2 4 8 16)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_list --threads=$thr --iterations=$its --yield=id --lists=4 >> lab2b_list.csv
	done
done

threads=(1 4 8 12 16)
iterations=(10 20 40 80)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_list --threads=$thr --iterations=$its --yield=id --lists=4 --sync=s >> lab2b_list.csv
		./lab2_list --threads=$thr --iterations=$its --yield=id --lists=4 --sync=m >> lab2b_list.csv
	done
done

threads=(1 2 4 8 12)
iterations=(1000)
lists=(4 8 16)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		for lst in "${lists[@]}"; do
			./lab2_list --threads=$thr --iterations=$its --lists=$lst --sync=s >> lab2b_list.csv
			./lab2_list --threads=$thr --iterations=$its --lists=$lst --sync=m >> lab2b_list.csv
		done
	done
done

./part2.gp