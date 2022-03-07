echo "testing read performance on 64 B"
date +"%T.%6N" 
for i in {1..100}; do cat 64_B > /dev/null ; done
date +"%T.%6N"



