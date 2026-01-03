#ifndef FUNCS_H
#define FUNCS_H

#include "aliases.h"
#include "nested.h"

// Using many types from other headers
using V2 = Vec2;
using V3 = Vec3;
using NodeAlias = DeepNode;

i32 increment(i32 x);
i32 multiply2(i32 x);

// A function using type aliases inside parameters
Calculator::UnaryFunc getUnaryFunc(bool flag);
Calculator::BinaryFunc getBinaryFunc(bool flag);

// A function using template alias
Ptr<i32> make_int_ptr(i32* p);

// A function using nested namespace alias
A::B::C::Wrapper<i32> make_wrapper(i32 v);

#endif
