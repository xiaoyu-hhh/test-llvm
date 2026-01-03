# 1 "macro.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "macro.cpp" 2
# 16 "macro.cpp"
class A {
  public:
  int A_sub(int a, int b) { return a - b; };
};

void printInt(const char *name, int value) {
  (void)name;
  (void)value;

}

int main() {
  int a = 3;
  int b = 5;
  int result1 = ((a + b) * (a + b));
  int result2 = ((a) > (b) ? (a) : (b));

  int num1 = 42;

  printInt("result1", result1);

  return 0;
}
