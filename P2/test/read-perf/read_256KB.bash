echo "testing read performance on 256 KB"
date +"%T.%6N" 
for i in {1..100}; do cat 256_KB > /dev/null ; done
date +"%T.%6N"



