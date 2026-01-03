
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