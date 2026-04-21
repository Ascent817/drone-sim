#pragma once

#include "../components/component.h"
#include "../components/transform.h"
#include "raylib.h"

class RotorBehavior : public Component {
 public:
  RotorBehavior(TransformComponent* transform, int activationKey,
                float spinSpeedRadPerSec = 20.0f)
      : transform(transform), activationKey(activationKey), spinSpeed(spinSpeedRadPerSec) {}

  void Update(float dt) override {
    if (transform == nullptr) return;
    if (!IsKeyDown(activationKey)) return;

    transform->Rotate(spinSpeed * dt, transform->up());
  }

 private:
  TransformComponent* transform = nullptr;
  int activationKey = KEY_NULL;
  float spinSpeed = 20.0f;
};
