
echo "testing make"

for i in {1..100}; do echo "start" ; date +"%T.%6N" ; cat Makefile ; cat functions.c ; cat functions.h ; cat main.c ; make ; echo "end" ; date +"%T.%6N"; make clean; done





