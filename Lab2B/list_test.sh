#~/bin/bash

rm -f lab2b_list.csv

#----------------------------------------------------------------------
#----------------------------------------------------------------------

#lab2b test 1

THREADS=(1 2 4 8 12 16 24)
ITERATIONS=(1000)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2b_list --threads=$k --iterations=$l --sync=m>> lab2b.csv
		./lab2b_list --threads=$k --iterations=$l --sync=s>> lab2b.csv
		./lab2b_add --threads=$k --iterations=$l --sync=m>> lab2b.csv
		./lab2b_add --threads=$k --iterations=$l --sync=s>> lab2b.csv

	done
done


#lab2b test 2

THREADS=(1 2 4 8 12 16 24)
ITERATIONS=(1000)
for k in ${THREADS[@]};
do 
	for l in ${ITERATIONS[@]};
	do
		./lab2b_list --threads=$k --iterations=$l --sync=m>> lab2b.csv
		./lab2b_list --threads=$k --iterations=$l --sync=s>> lab2b.csv
		./lab2b_add --threads=$k --iterations=$l --sync=m>> lab2b.csv
		./lab2b_add --threads=$k --iterations=$l --sync=s>> lab2b.csv

	done
done
