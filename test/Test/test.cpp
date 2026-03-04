template<typename T>
    class A {
public:
  T x;
  A(T _x) : x(_x) { }
  void foo() {}
};


int main(){
  A<int> k(1);
  k.foo();
  return 0;
}