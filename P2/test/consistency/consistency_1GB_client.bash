
echo "testing consistency on 1 GB"

for i in {1..5}; do echo "Iteration ${i}" | date +"%T.%6N" | cat 1_GB | date +"%T.%6N"; done




