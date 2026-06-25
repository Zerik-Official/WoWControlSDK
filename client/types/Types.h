#pragma once
#include <cstdint>

using guid_t = uint64_t;

template <typename T>
struct Vec3D { T x, y, z; };

struct VecXYZ : Vec3D<float> {};
