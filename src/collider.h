#pragma once

#include "component.h"
#include "raylib.h"

class Collider : public Component {
 public:
  enum class Type { Box, Mesh, Plane };

  virtual Type GetType() const = 0;

  Vector3 offset = {0.0f, 0.0f, 0.0f};
};

class BoxCollider : public Collider {
 public:
  Vector3 halfExtents = {0.5f, 0.5f, 0.5f};

  Type GetType() const override { return Type::Box; }
};

class MeshCollider : public Collider {
 public:
  Model* model = nullptr;

  Type GetType() const override { return Type::Mesh; }
};

class PlaneCollider : public Collider {
 public:
  Vector3 normal = {0.0f, 1.0f, 0.0f};
  float distance = 0.0f;

  Type GetType() const override { return Type::Plane; }
};
