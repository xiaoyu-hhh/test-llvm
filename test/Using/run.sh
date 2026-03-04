set -e

rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.out
rm -f *.o
~/iclang/llvm-project/build/bin/clang++ -iclang=ShareTest -c test2.cpp
