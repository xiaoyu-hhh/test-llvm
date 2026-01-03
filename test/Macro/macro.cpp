#define SQUARE(x) ((x) * (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MESSAGE "Macro example"
#define CONCAT(a, b) a##b
#define PRINT_INT(x) printInt(#x, x)
#define DEFINE_ADD_FUNCTION(name) \
int name(int a, int b) {          \
return a + b;                 \
}

#define DEFINE_SUB_FUNCTION(name) \
int name(int a, int b) {          \
return a - b;                 \
}

class A {
  public:
  DEFINE_SUB_FUNCTION(A_sub);
};

void printInt(const char *name, int value) {
  (void)name;
  (void)value;
  // Placeholder function for demonstration only
}

int main() {
  int a = 3;
  int b = 5;
  int result1 = SQUARE(a + b);   // Macro expansion: ((a + b) * (a + b))
  int result2 = MAX(a, b);       // Macro expansion: ((a) > (b) ? (a) : (b))

  int CONCAT(num, 1) = 42;       // Expands to: int num1 = 42;

  PRINT_INT(result1);            // Macro expansion with stringification

  return 0;
}

// /home/ygl/iclang/llvm-project/build/bin/clang++ -iclang=ShareTest -c macro.cpp