
echo "testing consistency on 4 KB"

for i in {1..100}; do echo "Iteration ${i}" | date +"%T.%6N" | cat 4_KB | date +"%T.%6N"; done




