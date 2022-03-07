
echo "testing write performance on 4 KB"
touch /tmp/afs/4_KB_copy
date +"%T.%6N" 
for i in {1..100}; do cp 4_KB 4_KB_copy ; done
date +"%T.%6N"



