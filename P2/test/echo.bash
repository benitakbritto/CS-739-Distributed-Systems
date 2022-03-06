
start=`date +%T.%6N`

for i in {1..100}; do echo "filename${i}.bmp"; done
end = `date +%T.%6N`

runtime=$((end-start))

