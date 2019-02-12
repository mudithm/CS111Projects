
#echo "This is a file" > test1_file.txt

# Test 1 - Bash / dash
#cat test1_file.txt | tr 'Tis' 'sti' | grep 'shti' | wc -c > out.t

# Test 1 - simpsh
#./simpsh --profiletotal --rdonly test1_file.txt --creat --rdwr out.t --pipe --pipe --command 0 3 1 tr 'Tis' 'sti' --command 2 5 1 grep 'shti' --command 4 1 1 wc -c --close 4


#test 2 - bash / dash
#echo "many lines" >> large_file.txt
#cat large_file | sed 's/abc/def/g' | tr 'def' 'abc' | wc -l | grep '5' > out.txt 2>err.txt


#test 2 - simpsh
#./simpsh --profiletotal --rdonly large_file.txt --wronly out.txt --pipe --pipe --pipe --wronly err.txt --command 0 3 8 sed 's/abc/def/' --command 2 5 8 tr 'def' 'abc' --command 4 7 8 wc -l --close 4 --command 6 1 8 grep '5'

#./simpsh --rdonly large_file.txt --wronly out.txt --pipe --pipe --pipe --wronly err.txt --command 0 3 8 sed 's/abc/def/' --command 2 5 8 tr 'def' 'abc' --command 4 7 8 wc -l --close 4 --command 6 1 8 grep '5'


#test 3 - bash / dash
sort < large_file | wc -l | grep "some word" > output.txt 2>err.txt

#test 3 - simpsh
#./simpsh --rdonly large_file.txt --creat --wronly output.txt --pipe --pipe --creat --wronly err.txt --command 0 3 6 sort --command 2 5 6 wc -l --close 2 --command 4 1 6 grep "some word" 

