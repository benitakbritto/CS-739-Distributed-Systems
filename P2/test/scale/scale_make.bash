
echo "testing make"
# doesn't work - can't use git clone on our fs
git clone https://github.com/mit-pdos/xv6-public.git 
# store a simple c file and make files in ~/files and make that
for i in {1..100}; do date +"%T.%6N" ; make ; date +"%T.%6N"; make clean; done





