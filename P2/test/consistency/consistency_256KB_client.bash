
echo "testing consistency on 256 KB"

for i in {1..100}; do echo "Iteration ${i}" | date +"%T.%6N" | cat 256_KB | date +"%T.%6N"; done




