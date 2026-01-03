# 1 "main.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "main.cpp" 2

using I32 = int;
using PtrI32 = int*;
using RefI32 = int&;
using FuncPtr = int(*)(int, int);

template<typename T, typename U>
using CondType = decltype(true ? T() : U());

struct A {
  void f(int);
  void f(double);
             ;
};

struct B : A {
  using A::f;
                     ;
};

int main() {

}
