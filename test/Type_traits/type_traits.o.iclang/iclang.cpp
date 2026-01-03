# 1 "type_traits.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "type_traits.cpp" 2
namespace std{

template<typename _Tp, typename _Up = _Tp&&>
_Up
__declval(int);

template<typename _Tp>
_Tp
__declval(long);


template<typename _Tp>
auto declval() noexcept -> decltype(__declval<_Tp>(0));
}


template<typename _Tp, typename _Up>
using __cond_t
    = decltype(true ? std::declval<_Tp>() : std::declval<_Up>());

__attribute__((deprecated)) int a();

template<typename T>
[[deprecated]] int b();

__attribute__((deprecated("bad"))) void foo2();

class demo{
public:
  [ [deprecated]] void a1();

};


int main(){

}
