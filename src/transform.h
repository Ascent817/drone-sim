#pragma once

#include "component.h"
#include "raymath.h"

class Transform : public Component {
 public:
  Matrix matrix = MatrixIdentity();

  Vector3 GetPosition() const {
    return {matrix.m12, matrix.m13, matrix.m14};
  }

  //
};