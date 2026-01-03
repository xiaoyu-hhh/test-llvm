rm -rf *.iclang
rm -rf *.iclangtmp
rm -f *.o
clang -c type_traits.cpp
echo "origin clang -c is ok"
clang -Xclang -ast-dump -c type_traits.cpp> type_traits.txt
/home/ygl/iclang/llvm-project/build/bin/clang -iclang=ShareTest -c type_traits.cpp
echo "iclang=ShareTest -c is ok"