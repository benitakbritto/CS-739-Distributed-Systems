
echo "testing scanDir"

mkdir -p a; mkdir -p a/b; mkdir -p a/b/c; mkdir -p a/b/c/d; mkdir -p a/b/c/d/e;
touch a/a.txt; touch a/b/b.txt; touch a/b/c/c.txt; touch a/b/c/d/d.txt; touch a/b/c/d/e/e.txt;

for i in {1..100}; do date +"%T.%6N" ; ls a; stat a/a.txt ;ls a/b; stat a/b/b.txt ;ls a/b/c; stat a/b/c/c.txt ; ls a/b/c/d; stat a/b/c/d/d.txt ;ls a/b/c/d/e; stat a/b/c/d/e/e.txt ;date +"%T.%6N"; done




