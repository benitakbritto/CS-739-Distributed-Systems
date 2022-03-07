
echo "testing write performance on 64 B"
touch /tmp/afs/64_B_copy
date +"%T.%6N" 
for i in {1..100}; do cp 64_B 64_B_copy ; done
date +"%T.%6N"



