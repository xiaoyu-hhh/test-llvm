# 1 "extern_Cxx.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "extern_Cxx.cpp" 2


                                                         

extern "C" {
  inline int inline_func2(int a, int b) {
    return a + b;
  }
}

extern "C++" {
                                                              
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
                              
                                                             
                                                

private:
  int value;
};

int main() {

  return 0;
}
