# 1 "default.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "default.cpp" 2


class Demo {
public:

  Demo() = default;




  ~Demo() {
    value--;
  }


  Demo(const Demo&) = default;


  Demo& operator=(const Demo&) = default;


  Demo(Demo&&) = default;


  Demo& operator=(Demo&&) = default;

  Demo(const int x) : value(x) {}


private:
  int value;
};

class A {
public:
  Demo demo;
};

int main() {
  Demo a;
  Demo b(1);
  Demo c = a;
  Demo d(a);
  d = b;
  {
    Demo e;
  }
  A test;

  return 0;
}
