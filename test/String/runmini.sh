rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.o
rm -f *.out
clang mini.cpp
echo "origin clang is ok"
clang -c mini.cpp
echo "origin clang -c is ok"
clang -Xclang -ast-dump -c mini.cpp > mini.txt
/home/ygl/iclang/llvm-project/build/bin/clang -iclang=ShareTest -c mini.cpp
echo "iclang=ShareTest -c is ok"