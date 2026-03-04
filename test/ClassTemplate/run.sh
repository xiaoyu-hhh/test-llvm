set -e

rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.out
rm -f *.o
~/iclang/llvm-project/build/bin/clang++ -iclang=ShareTest -c test1.cpp
echo "ok"