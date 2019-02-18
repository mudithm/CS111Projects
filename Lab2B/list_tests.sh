#!/bin/bash

rm -f lab2_list.csv

# lab_list 1
THREADS=(1)
ITERATIONS=(10 100 1000 10000 20000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_list --threads=$k --iterations=$l >> lab2_list.csv
	done
done


# lab_list 2, part 1
THREADS=(2 4 8 12)
ITERATIONS=(1 10 100 1000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_list --threads=$k --iterations=$l >> lab2_list.csv
	done
done

# lab_list 2, part 2
THREADS=(2 4 8 12)
ITERATIONS=(1 2 4 8 16 32)
YIELDS=(i d il dl)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		for y in ${YIELDS[@]};
		do
			./lab2_list --threads=$k --iterations=$l --yield=$y>> lab2_list.csv
		done	
	done
done


# lab_list 3
THREADS=(12)
ITERATIONS=(32)
YIELDS=(i d il dl)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		for y in ${YIELDS[@]};
		do
			./lab2_list --threads=$k --iterations=$l --yield=$y --sync=m>> lab2_list.csv
			./lab2_list --threads=$k --iterations=$l --yield=$y --sync=s>> lab2_list.csv
		done	
	done
done


# lab_list 4
THREADS=(1 2 4 8 12 16 24)
ITERATIONS=(1000)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
			./lab2_list --threads=$k --iterations=$l --sync=m>> lab2_list.csv
			./lab2_list --threads=$k --iterations=$l --sync=s>> lab2_list.csv
	done
done




