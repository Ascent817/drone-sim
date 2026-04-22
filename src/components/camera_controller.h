#pragma once

#include <cmath>

#include "component.h"
#include "raylib.h"
#include "raymath.h"

class CameraController : public Component {
 public:
  Camera3D camera{};
  float yaw = 0.0f;
  float pitch = 0.0f;
  bool rotating = false;

  ~CameraController() {
    if (rotating) EnableCursor();
  }

  static constexpr float kLookSensitivity = 0.0035f;
  static constexpr float kMoveSpeed = 6.0f;

  void Start() override { InitFromCamera(); }

  void InitFromCamera() {
    Vector3 forward =
        Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    yaw = atan2f(forward.x, forward.z);
    pitch = asinf(forward.y);
    rotating = false;
  }

  void UpdateLook() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      DisableCursor();
      rotating = true;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
      EnableCursor();
      rotating = false;
    }

    if (!rotating) return;

    Vector2 mouseDelta = GetMouseDelta();
    yaw -= mouseDelta.x * kLookSensitivity;
    pitch -= mouseDelta.y * kLookSensitivity;
    pitch = Clamp(pitch, -1.5f, 1.5f);

    Vector3 forward = {
        sinf(yaw) * cosf(pitch),
        sinf(pitch),
        cosf(yaw) * cosf(pitch),
    };
    camera.target = Vector3Add(camera.position, forward);
  }

  void UpdateMovement() {
    Vector3 forward =
        Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 flatForward =
        Vector3Normalize(Vector3{forward.x, 0.0f, forward.z});
    if (Vector3Length(flatForward) < 0.0001f)
      flatForward = Vector3{0.0f, 0.0f, 1.0f};
    Vector3 right =
        Vector3Normalize(Vector3CrossProduct(flatForward, camera.up));

    Vector3 movement{};
    if (IsKeyDown(KEY_W)) movement = Vector3Add(movement, flatForward);
    if (IsKeyDown(KEY_S)) movement = Vector3Subtract(movement, flatForward);
    if (IsKeyDown(KEY_D)) movement = Vector3Add(movement, right);
    if (IsKeyDown(KEY_A)) movement = Vector3Subtract(movement, right);
    if (IsKeyDown(KEY_SPACE)) movement.y += 1.0f;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
      movement.y -= 1.0f;

    if (Vector3Length(movement) < 0.0001f) return;

    movement =
        Vector3Scale(Vector3Normalize(movement), kMoveSpeed * GetFrameTime());
    camera.position = Vector3Add(camera.position, movement);
    camera.target = Vector3Add(camera.target, movement);
  }

  void Update(float /*dt*/) override {
    UpdateMovement();
    UpdateLook();
  }
};