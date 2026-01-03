# 1 "mini.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 480 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "mini.cpp" 2
class A{
public:
  void foo() {

  }
};

template <typename T>
int b() {
  return 0;
}

namespace std
{

template <typename T>
  constexpr int __is_complete_or_unbounded(T)
{ b<T>();
  A a;
  a.foo();
  return {};
}

template <typename _TypeIdentity,
    typename _NestedType = typename _TypeIdentity::type>
  constexpr int __is_complete_or_unbounded(_TypeIdentity)
{ return {}; }


template<typename T>
  struct is_trivial
{
  static_assert(std::__is_complete_or_unbounded(T{}));
};
}


int main() {
  return 0;
}
