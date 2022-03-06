
echo "testing write performance on 256 KB"
touch 256_KB
date +"%T.%6N" 
for i in {1..100}; do cp 256_KB 256_KB_copy ; done
date +"%T.%6N"



