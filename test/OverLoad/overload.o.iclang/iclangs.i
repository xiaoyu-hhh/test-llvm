# 1 "overload.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "overload.cpp" 2




class A {
  public:
  void foo(int x) {
    int a = x + 1;
  }

  void foo(double x) {
    int b = (int)x;
  }

                                                                            
};

template<typename T>
void bar(T t) {
  A a;
  a.foo(t);
}


class B : public A {
public:
  void xxx(){
    bar(2);
    foo(2.2);
  }
};


int main() {
  B b;
  b.xxx();
  return 0;
}
