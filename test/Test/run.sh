set -e

rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.out
rm -f *.o
export ShareCollection=1
/home/ygl/iclang/llvm-project/build/bin/clang -iclang=ShareTest -c test.cpp