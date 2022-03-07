
echo "testing scanDir"
git clone https://github.com/mit-pdos/xv6-public.git
for i in {1..10}; do date +"%T.%6N" | make | date +"%T.%6N"| make clean; done




