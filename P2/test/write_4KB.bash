
echo "testing write performance on 4 KB"
touch 4_KB
date +"%T.%6N" 
for i in {1..100}; do cp 4_KB 4_KB_copy ; done
date +"%T.%6N"



