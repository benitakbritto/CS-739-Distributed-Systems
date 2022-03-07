
echo "testing consistency on 16 MB"

for i in {1..100}; do echo "Iteration ${i}" | date +"%T.%6N" | cat 16_MB | date +"%T.%6N"; done

echo a > 16_MB



