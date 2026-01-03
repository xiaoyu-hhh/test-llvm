set -e

rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.o
clang++ -c str.cpp
echo "origin clang -c is ok"
/home/ygl/iclang/llvm-project/build/bin/clang -iclang=ShareTest -c str.cpp
echo "iclang=ShareTest -c is ok"
