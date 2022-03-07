echo "testing read performance on 4 KB"
date +"%T.%6N" 
for i in {1..100}; do cat 4_KB > /dev/null ; done
date +"%T.%6N"



