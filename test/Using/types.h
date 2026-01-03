#ifndef TYPES_H
#define TYPES_H

// Basic primitive type aliases
using i8 = signed char;
using i16 = short;
using i32 = int;
using i64 = long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long;

// Alias chain
using byte = u8;
using word = u16;
using dword = u32;

// A simple struct
struct Vec2 {
  i32 x;
  i32 y;
};

// Struct with inner type aliases
struct Vec3 {
  using value_type = i32;
  value_type x;
  value_type y;
  value_type z;
};

// Template struct
template<typename T>
struct Box {
  using value_type = T;
  T v;
};

// Template alias
template<typename T>
using Ptr = T*;

#endif
