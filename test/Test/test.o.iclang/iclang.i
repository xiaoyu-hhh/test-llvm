# 1 "test.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "test.cpp" 2

template <typename T>
void foo(T x) {
  return;
}

template <typename T>
void bar(T x) {
  return;
}

template <typename T>
void func(T x) {
  return;
}

class A {
  public:
  template <typename T>
    void bar(T x) {
    return;
  }
  void foo(int x) {};
};


void global(int x) {};

int main(){

}
