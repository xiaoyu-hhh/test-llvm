#ifndef ALIASES_H
#define ALIASES_H

#include "types.h"

// Namespace with aliases
namespace Math {
using Coord = i32;
using Point2 = Vec2;
using Point3 = Vec3;
using PointPtr = Ptr<Point3>;

// Template alias inside namespace
template<typename T>
using BoxPtr = Box<T>*;
}

// Nested namespace alias
namespace M = Math;

// A struct demonstrating using declarations and function type aliases
struct Calculator {

  // Function type aliases
  using UnaryFunc = i32 (*)(i32);
  using BinaryFunc = i32 (*)(i32, i32);

  // Bring types from another scope
  using Coord = Math::Coord;
  using P2 = Math::Point2;

  Coord mul(Coord a, Coord b);
  Coord add(Coord a, Coord b);
};

#endif
