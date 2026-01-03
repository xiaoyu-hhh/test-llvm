# 1 "attr.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "attr.cpp" 2
int z;[ [ noreturn]] void a();
int y; [ [ nodiscard]] int b() = delete;
int x;__attribute__((deprecated)) int c();
int w;__attribute__((deprecated)) int d();
int v;template<typename T>
[[deprecated]] int e();
int t;[[deprecated("bad")]] int f();
int s;__attribute__((deprecated("bad"))) void g();
class demo{
public:
  int a;[[noreturn]] void a1();
  int b;[[noreturn]] void a2() = delete;
  int c;[[deprecated("bad")]] int a3();
  int d;[[noreturn]] static void a4();
};






int main(){

  return 0;
}
