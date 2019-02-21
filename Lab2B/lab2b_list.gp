#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. average wait time per operation
#
# output:
#	lab2b_1.png ... throughput vs number of threads for mutex and spin-lock synched operations
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations
#	lab2b_3.png ... successful iterations vs. threads for each synchronization method
#	lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists
#	lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists

# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# test 1
set title "List-1: Total Throughput vs Number of Threads with Synchronization"
set xlabel "Threads"
set xrange [0.75:]
set logscale x 2
set ylabel "Throughput (Operations per Second)"
set logscale y 10
set output 'lab2b_1.png'

# grep out successful protected list runs
plot \
      "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list operations w/ mutex' with linespoints lc rgb 'red', \
      "< grep 'list-none-s,[0-9]\\+,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list operations w/ spin lock' with linespoints lc rgb 'blue'
 

# test 2
set title "List-2: Mean Operation and Wait Times for Mutex-locked operations"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Mean Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep -E \"list-none-m,[0-9]+,1000,1,\" lab2b_list.csv" using ($2):($7) \
	title 'Time per Operation' with linespoints lc rgb 'red', \
	"< grep -E \"list-none-m,[0-9]+,1000,1,\" lab2b_list.csv" using ($2):($8) \
	title 'Wait time' with linespoints lc rgb 'green'



     
# test 3
set title "List-3: Successful Iterations vs Number of Threads"
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# ...
plot \
    "< grep 'list-id-none,[0-9]\\+,[0-9]\\+,4' lab2b_list.csv" using ($2):($3) \
	title 'No synchronization' with points lc rgb 'red', \
	 "< grep 'list-id-s,[0-9]\\+,[0-9]\\+,4' lab2b_list.csv" using ($2):($3) \
	title 'spin-locked' with points lc rgb 'green', \
	 "< grep 'list-id-m,[0-9]\\+,[0-9]\\+,4' lab2b_list.csv" using ($2):($3) \
	title 'mutex-locked' with points lc rgb 'blue'




# test 4
set title "List-4: Throughput vs Number of Threads for Mutex-locked partitioned lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (Operations per Second)"
set logscale y
unset yrange
set output 'lab2b_4.png'
plot \
    "< grep -E \"list-none-m,[0-9]+,1000,1,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 sublist' with linespoints lc rgb 'red', \
	"< grep -E \"list-none-m,[0-9]+,1000,4,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 sublists' with linespoints lc rgb 'green', \
	"< grep -E \"list-none-m,[0-9]+,1000,8,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 sublists' with linespoints lc rgb 'blue', \
	"< grep -E \"list-none-m,[0-9]+,1000,16,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 sublists' with linespoints lc rgb 'orange', \


# test 5
set title "List-5: Throughput vs Number of Threads for Spin-locked partitioned lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (Operations per Second)"
set logscale y
unset yrange
set output 'lab2b_5.png'
plot \
    "< grep -E \"list-none-s,[0-9]+,1000,1,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 sublist' with linespoints lc rgb 'red', \
	"< grep -E \"list-none-s,[0-9]+,1000,4,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 sublists' with linespoints lc rgb 'green', \
	"< grep -E \"list-none-s,[0-9]+,1000,8,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 sublists' with linespoints lc rgb 'blue', \
	"< grep -E \"list-none-s,[0-9]+,1000,16,\" lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 sublists' with linespoints lc rgb 'orange', \
