set -e

rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.o
clang++ -c stri.cpp
/home/ygl/iclang/llvm-project/build/bin/clang -iclang=ShareTest -c stri.cpp
echo "Done"
