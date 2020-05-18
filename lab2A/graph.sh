#!/bin/bash

rm lab2_add.csv
rm lab2_list.csv


threads=(2 4 8 12)
iterations=(100 1000 10000 100000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_add --threads=$thr --iterations=$its >> lab2_add.csv
	done
done
threads=(2 4 8 12)
iterations=(10 20 40 80 100 1000 10000 100000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_add --threads=$thr --iterations=$its --yield >> lab2_add.csv
	done
done
threads=(1 2 4)
iterations=(100 1000 10000 100000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_add --threads=$thr --iterations=$its >> lab2_add.csv
		./lab2_add --threads=$thr --iterations=$its --yield >> lab2_add.csv
	done
done
threads=(1 2 4 8 12)
iterations=(10000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_add --threads=$thr --iterations=$its --yield --sync=m >> lab2_add.csv
		./lab2_add --threads=$thr --iterations=$its --yield --sync=c >> lab2_add.csv
		./lab2_add --threads=$thr --iterations=$its --yield --sync=s >> lab2_add.csv
		./lab2_add --threads=$thr --iterations=$its --sync=m >> lab2_add.csv
		./lab2_add --threads=$thr --iterations=$its --sync=c >> lab2_add.csv
		./lab2_add --threads=$thr --iterations=$its --sync=s >> lab2_add.csv
	done
done


./lab2_list --threads=1  --iterations=10000 >> lab2_list.csv
./lab2_list --threads=1  --iterations=20000	>> lab2_list.csv
threads=(1 2 4 8 12)
iterations=(10 100 1000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_list --threads=$thr --iterations=$its >> lab2_list.csv
	done
done
threads=(2 4 8 12)
iterations=(2 4 8 12 16 24)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_list --threads=$thr --iterations=$its --yield=i>> lab2_list.csv
		./lab2_list --threads=$thr --iterations=$its --yield=d>> lab2_list.csv
		./lab2_list --threads=$thr --iterations=$its --yield=il>> lab2_list.csv
		./lab2_list --threads=$thr --iterations=$its --yield=dl>> lab2_list.csv
	done
done
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 >> lab2_list.csv
threads=(1 2 4 8 12 16 24)
iterations=(1000)
for thr in "${threads[@]}"; do
	for its in "${iterations[@]}"; do
		./lab2_list --threads=$thr --iterations=$its --sync=m >> lab2_list.csv
		./lab2_list --threads=$thr --iterations=$its --sync=s >> lab2_list.csv
	done
done
./part1.gp
./part2.gp
