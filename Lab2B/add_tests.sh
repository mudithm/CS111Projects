#!/bin/bash

rm -f lab2_add.csv

# lab_add 1
THREADS=(2 4 8 12)
ITERATIONS=(10 20 40 80 100 1000 10000 100000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l >> lab2_add.csv
	done
done

# lab_add 2
THREADS=(2 8)
ITERATIONS=(100 1000 10000 100000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --threads=$k --iterations=$l >> lab2_add.csv		
		./lab2_add --yield --threads=$k --iterations=$l >> lab2_add.csv
	done
done

# lab_add 3
THREADS=(1)
ITERATIONS=(10 20 40 80 100 1000 10000 100000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --threads=$k --iterations=$l >> lab2_add.csv		
	done
done

#lab_add 4, mutex and CAS
THREADS=(2 4 8 12)
ITERATIONS=(10000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l --sync=m>> lab2_add.csv		
		./lab2_add --yield --threads=$k --iterations=$l --sync=c>> lab2_add.csv
	done
done

#lab_add 4, spin
THREADS=(2 4 8 12)
ITERATIONS=(1000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l --sync=s>> lab2_add.csv
	done
done


#lab_add 5, unprotected, mutex, spun and CAS
THREADS=(1 2 4 8 12)
ITERATIONS=(10000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --threads=$k --iterations=$l --sync=m>> lab2_add.csv		
		./lab2_add --threads=$k --iterations=$l --sync=c>> lab2_add.csv		
		./lab2_add --threads=$k --iterations=$l >> lab2_add.csv
		./lab2_add  --threads=$k --iterations=$l --sync=s>> lab2_add.csv
	done
done

