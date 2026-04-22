#pragma once

#include "../components/component.h"
#include "../components/physics_body.h"
#include "../components/transform.h"
#include "../entity.h"
#include "../utils/conversions.h"
#include "raylib.h"

class RotorBehavior : public Component {
 public:
  RotorBehavior(int activationKey, float spinSpeedRadPerSec = 40.0f,
                float thrustPerRadPerSec = 0.05f)
      : activationKey(activationKey),
        spinSpeed(spinSpeedRadPerSec),
        thrustPerSpeed(thrustPerRadPerSec) {}

  void Start() override {
    Entity* owner = GetEntity();
    if (owner == nullptr) return;
    transform = owner->GetComponent<TransformComponent>();
    droneBody = owner->GetParent()->GetComponent<PhysicsBody>();
  }

  void Update(float dt) override {
    if (transform == nullptr || droneBody == nullptr || droneBody->body == nullptr) return;
    if (!IsKeyDown(activationKey)) return;

    transform->Rotate(spinSpeed * dt, transform->up());

    const Vector3 rotorLocalPos = transform->position();
    const float thrust = spinSpeed * thrustPerSpeed;
    droneBody->body->applyLocalForceAtLocalPosition(
        ToRp3d(Vector3Scale(transform->up(), thrust)),
        rp3d::Vector3(rotorLocalPos.x, rotorLocalPos.y, rotorLocalPos.z));
  }

 private:
  TransformComponent* transform = nullptr;
  PhysicsBody* droneBody = nullptr;

  int activationKey = KEY_NULL;
  float spinSpeed = 40.0f;
  float thrustPerSpeed = 0.12f;
};
