echo "testing read performance on 1_GB"
date +"%T.%6N" 
for i in {1..5}; do cat 1_GB > /dev/null ; done
date +"%T.%6N"



