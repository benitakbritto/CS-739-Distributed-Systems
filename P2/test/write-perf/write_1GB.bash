
echo "testing write performance on 1 GB"
touch /tmp/afs/1_GB_copy
date +"%T.%6N" 
for i in {1..5}; do cp 1_GB 1_GB_copy ; done
date +"%T.%6N"



