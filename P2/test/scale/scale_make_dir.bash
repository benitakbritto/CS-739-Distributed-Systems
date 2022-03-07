
echo "testing scalability"

for i in {1..100}; do date +"%T.%6N" | mkdir a;a/b;a/b/c;a/b/c/d;a/b/c/d/e | date +"%T.%6N"| rmdir a;a/b;a/b/c;a/b/c/d;a/b/c/d/e; done

mkdir a;a/b;a/b/c;a/b/c/d;a/b/c/d/e


