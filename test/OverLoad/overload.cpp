// test_overload.cpp
// 不包含头文件，避免噪音符号

// 定义三个重载函数
class A {
  public:
  void foo(int x) {
    int a = x + 1;
  }

  void foo(double x) {
    int b = (int)x;
  }

  void foo(const char* s) {
    int c = 0;
    while (s[c] != '\0') ++c;
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

// main函数只调用其中一个
int main() {
  B b;
  b.xxx();
  return 0;
}
