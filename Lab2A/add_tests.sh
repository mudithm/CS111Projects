#~/bin/bash

rm lab2_add.csv

iter=100
for i in `seq 1 10`;
do 
	for j in `seq 1 4`;
	do
		./lab2_add --threads=$i --iterations=$iter >> lab2_add.csv
		(( iter = 10 * $iter ))
	done
	iter=100
done

THREADS=(2 4 8 12)
ITERATIONS=(10 20 40 80 100 1000 10000 100000)

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l >> lab2_add.csv
	done
done


for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --threads=$k --iterations=$l --sync=m >>  lab2_add.csv
	done
done

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l --sync=m  >>  lab2_add.csv
	done
done

THREADS2=(2 4 6)
ITERATIONS2=(10 20 40 80 100 1000)

for k in ${THREADS2[@]};
do 
	for l in ${ITERATIONS2[@]};
	do
		./lab2_add --threads=$k --iterations=$l --sync=s  >>  lab2_add.csv
	done
done

for k in ${THREADS2[@]};
do 
	for l in ${ITERATIONS2[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l --sync=s  >>  lab2_add.csv
	done
done



for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --threads=$k --iterations=$l --sync=c  >>  lab2_add.csv
	done
done

for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2_add --yield --threads=$k --iterations=$l --sync=c  >>  lab2_add.csv
	done
done
