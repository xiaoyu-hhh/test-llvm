

class Demo {
public:
  // Explicitly enable default constructor
  Demo() = default;

  // should we clear this?
  // int x;~Demo() = default;int y;

  ~Demo() {
    value--;
  }

  // Explicit copy constructor
  Demo(const Demo&) = default;

  // Explicit copy assignment
  Demo& operator=(const Demo&) = default;

  // Explicit move constructor
  Demo(Demo&&) = default;

  // Explicit move assignment
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
  Demo a;            // default constructor
  Demo b(1);
  Demo c = a;        // copy constructor
  Demo d(a);
  d = b;
  {
    Demo e;
  }
  A test;
  // // Demo d = std::move(b);  // move constructor
  return 0;
}

// /home/ygl/iclang/llvm-project/build/bin/clang++ -iclang=ShareTest -c default.cpp
