#pragma once

#include <reactphysics3d/reactphysics3d.h>

#include "raylib.h"
#include "raymath.h"

inline rp3d::Vector3 ToRp3d(const Vector3& v) {
  return rp3d::Vector3(v.x, v.y, v.z);
}

inline Vector3 ToRaylib(const rp3d::Vector3& v) {
  return Vector3{v.x, v.y, v.z};
}