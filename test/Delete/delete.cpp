
inline void jjj(double &x) = delete;

namespace ns{
void foo(double &x) = delete;

template<typename T>
void launder(T a) = delete;
}

extern "C++"{
template<typename T>
void launder2(T a) = delete;

void foo2(double &x) = delete;
}
template<typename T>
void launder3(T a) = delete;


void global(int);

void temp() {
  global(10);
}

void global(int b) {
  int a = b;
}

class demo{
public:
  void bar(int x) = delete;
  void foo(double y) = delete;
  template<typename T>
  void hhh(T a) = delete;int x;
};

int main() {
  return 0;
}
