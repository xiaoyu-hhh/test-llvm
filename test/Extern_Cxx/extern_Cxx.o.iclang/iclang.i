# 1 "extern_Cxx.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "extern_Cxx.cpp" 2


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

  return 0;
}
