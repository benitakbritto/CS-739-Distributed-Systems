
echo "testing makedir"

for i in {1..100}; do date +"%T.%6N"; mkdir -p a; mkdir -p a/b; mkdir -p a/b/c; mkdir -p a/b/c/d; mkdir -p a/b/c/d/e; date +"%T.%6N"; rm -rf a; done


