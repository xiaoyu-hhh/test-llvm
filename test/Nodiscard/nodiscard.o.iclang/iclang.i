# 1 "nodiscard.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "nodiscard.cpp" 2
# 22 "nodiscard.cpp"
class Test {
  public:
  [[nodiscard]] int foo() { return 0; }
  [[nodiscard]] inline int bar(){
    return 0;
  }
};

int main() {

  return 0;
}
