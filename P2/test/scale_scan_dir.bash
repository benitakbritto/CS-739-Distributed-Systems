
echo "testing scanDir"

for i in {1..100}; do date +"%T.%6N" | ls a| stat a.txt |ls a/b| stat b.txt |ls a/b/c| stat c.txt | ls a/b/c/d| stat d.txt |ls a/b/c/d/e| stat e.txt |date +"%T.%6N"; done




