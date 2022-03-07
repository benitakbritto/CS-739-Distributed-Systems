echo "testing read performance on 16 MB"
date +"%T.%6N" 
for i in {1..100}; do cat 16_MB > /dev/null ; done
date +"%T.%6N"



