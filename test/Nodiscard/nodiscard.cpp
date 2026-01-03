
//namespace demo {
//[[nodiscard]] int foo() {
//  return 0;
//}
//
//template<typename T>
//  [[nodiscard]] T Return(T a) { return a; }
//
//}
//
//extern "C++" {
//  [[nodiscard]] int bar();
//  [[nodiscard]] inline int foo(){
//    return 0;
//  }
//  template<typename T>
//    [[nodiscard]] T _add(T a, T b) { return a + b; }
//
//}

class Test {
  public:
  [[nodiscard]] int foo() { return 0; }
  [[nodiscard]] inline int bar(){
    return 0;
  }
};

int main() {
  // foo();
  return 0;
}
