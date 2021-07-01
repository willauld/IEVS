#rm ./test01.out
./a.exe test01ini.ini > ./test01ini.out 
#<<!
#12345
#3
#!
diff -s ./test01ini.out ./test01.good