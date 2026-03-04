# 1 "test1.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "test1.cpp" 2
class B {
public:
  B(int x = 0)
  { }

};

template<typename T>
class A : public B{
public:
  A(T x) : B(x)
  { }
};

int main(){
  A<int> a(1);
  return 0;
}
