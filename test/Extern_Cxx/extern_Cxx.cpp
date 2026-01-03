// /home/ygl/iclang/llvm-project/build/bin/clang -c extern_Cxx.cpp -iclang=ShareTest

inline int inline_func1(int a, int b) {
  return a + b;
}

extern "C" {
  inline int inline_func2(int a, int b) {
    return a + b;
  }
}

extern "C++" {
  inline int inline_func(int a, int b) {
    return a + b;
  }
  [[__nodiscard__]] inline int foo(){
    return 0;
  }

}

extern "C++" {
  int addNumbers(int a, int b) {
    return a + b;
  }
}

class MyClass {
public:
  MyClass(int x) : value(x) {}
  void normalCppFunction() {
    value = addNumbers(1,2);
  }
  [[__nodiscard__]] int foo(){
    return 0;
  }

private:
  int value;
};

int main() {
//  int c = inline_func(2,3);
  return 0;
}
