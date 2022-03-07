
echo "testing consistency on 64 B"

for i in {1..100}; do echo "Iteration ${i}" | date +"%T.%6N" | cat 64_B | date +"%T.%6N"; done




