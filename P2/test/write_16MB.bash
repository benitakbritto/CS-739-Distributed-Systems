
echo "testing write performance on 16 MB"
touch 16_MB_copy
date +"%T.%6N" 
for i in {1..100}; do cp 16_MB 16_MB_copy ; done
date +"%T.%6N"



