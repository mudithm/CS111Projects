#~/bin/bash

rm -f lab2b_list.csv

#----------------------------------------------------------------------
#----------------------------------------------------------------------

#lab2b test 1 and 2

THREADS=(1 2 4 8 12 16 24)
ITERATIONS=(1000)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2b_list --threads=$k --iterations=$l --sync=m>> lab2b_list.csv
		./lab2b_list --threads=$k --iterations=$l --sync=s>> lab2b_list.csv
	done
done

#lab2b test 3

THREADS=(1 4 8 12 16)
ITERATIONS=(1 2 4 8 16)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2b_list --threads=$k --iterations=$l --yield=id --lists=4 >> lab2b_list.csv
	done
done

THREADS=(1 4 8 12 16)
ITERATIONS=(10 20 40 80)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2b_list --threads=$k --iterations=$l --yield=id --lists=4 --sync=s >> lab2b_list.csv
		./lab2b_list --threads=$k --iterations=$l --yield=id --lists=4 --sync=m >> lab2b_list.csv	done
	done
done
#lab2b test 4 -- mutex

THREADS=(1 2 4 8 12)
ITERATIONS=(1000)
LISTS=(4 8 16)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		for m in ${LISTS[@]};
		do
			./lab2b_list --threads=$k --iterations=$l --lists=$m --sync=m >> lab2b_list.csv
		done
	done
done

#lab2b test 5 -- spin


THREADS=(1 2 4 8 12)
ITERATIONS=(1000)
LISTS=(4 8 16)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		for m in ${LISTS[@]};
		do
			./lab2b_list --threads=$k --iterations=$l --lists=$m --sync=s >> lab2b_list.csv
		done
	done
done