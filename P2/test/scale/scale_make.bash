
echo "testing make"

date +"%T.%6N"
for i in {1..100}; do cp Makefile Makefile_copy ; cp functions.c functions_copy.c ; cp functions.h functions_copy.h ; cp main.c main_copy.c ; make ; touch functions.c ;  done
date +"%T.%6N"




