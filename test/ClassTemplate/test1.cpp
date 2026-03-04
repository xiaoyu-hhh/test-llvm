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