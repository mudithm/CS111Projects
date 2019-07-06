#!/bin/bash

# modified version of the simple checks provided in P4B_check.sh
echo "bad argument check"
./lab4b --asdf < /dev/tty > /dev/null 2>STDERR
if [ ! -s STDERR ]
then
    echo "no stderr message for bad argument"
else
    echo ""
fi

echo "argment check"
rm -f file_specifically_for_testing
./lab4b --period=3--scale=C --log=file_specifically_for_testing <<-EOF
SCALE=F
PERIOD=1
START
STOP
LOG something
OFF
EOF
ret=$?

if [ $ret -ne 0 ]
then
    echo "done messed up"
fi

if [ ! -s file_specifically_for_testing ]
then
    echo "log file not created"
else
    echo "commands logged and supported"
    cat file_specifically_for_testing
fi

rm -f file_specifically_for_testing
rm -f STDERR
