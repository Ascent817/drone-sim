#pragma once

#include <reactphysics3d/reactphysics3d.h>

#include "component.h"
#include "physics_world.h"

class PhysicsBody : public Component {
 public:
  rp3d::RigidBody* body = nullptr;

  Matrix GetTransformMatrix() const {
    return Rp3dTransformToMatrix(body->getTransform());
  }

  Vector3 GetPosition() const {
    auto p = body->getTransform().getPosition();
    return {p.x, p.y, p.z};
  }
};
