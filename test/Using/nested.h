#ifndef NESTED_H
#define NESTED_H

#include "types.h"

// Deeply nested namespaces
namespace A {
namespace B {
namespace C {

struct Node {
  using index_t = u32;
  index_t id;
};

// using declaration pulling parent namespace type
using NodeId = Node::index_t;

// Template inside nested namespace
template<typename T>
struct Wrapper {
  using inner_t = T;
  T data;
};

} // namespace C
} // namespace B
} // namespace A

// Alias for deeply nested type
using DeepNode = A::B::C::Node;
using DeepWrapperInt = A::B::C::Wrapper<i32>;

#endif
