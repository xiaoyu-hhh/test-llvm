
using I32 = int;
using PtrI32 = int*;
using RefI32 = int&;
using FuncPtr = int(*)(int, int);

template<typename T, typename U>
using CondType = decltype(true ? T() : U());

struct A {
  void f(int);
  void f(double);
  void k(int);
};

struct B : A {
  using A::f;   // bring A::f overloads into B
  void f(const char*);
};

int main() {

}
