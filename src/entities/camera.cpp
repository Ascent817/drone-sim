#include "camera.h"

#include "../components/transform.h"

namespace {
Vector3 ExtractWorldPosition(const Matrix& m) {
  return Vector3{m.m12, m.m13, m.m14};
}

Vector3 ExtractForward(const Matrix& m) {
  return Vector3Normalize(Vector3{m.m8, m.m9, m.m10});
}
}  // namespace

Entity* CreateCameraEntity(World& world, Vector3 position, Vector3 target) {
  Entity* e = world.CreateEntity();

  auto* cc = e->AddComponent<CameraController>();
  cc->camera.position = position;
  cc->camera.target = target;
  cc->camera.up = {0.0f, 1.0f, 0.0f};
  cc->camera.fovy = 45.0f;
  cc->camera.projection = CAMERA_PERSPECTIVE;

  return e;
}

Entity* CreateDroneCameraEntity(Entity* drone, Vector3 localPosition) {
  if (drone == nullptr) return nullptr;

  Entity* e = drone->AddChild();
  if (auto* tc = e->GetComponent<TransformComponent>()) {
    tc->setPosition(localPosition);
  }

  return e;
}

Camera3D BuildCameraFromEntityTransform(const Entity* cameraEntity, float fovy) {
  Camera3D camera{};
  camera.fovy = fovy;
  camera.projection = CAMERA_PERSPECTIVE;
  camera.up = {0.0f, 1.0f, 0.0f};

  if (cameraEntity == nullptr) {
    camera.position = {0.0f, 2.0f, -5.0f};
    camera.target = {0.0f, 1.0f, 0.0f};
    return camera;
  }

  const auto* tc = cameraEntity->GetComponent<TransformComponent>();
  if (tc == nullptr) {
    camera.position = {0.0f, 2.0f, -5.0f};
    camera.target = {0.0f, 1.0f, 0.0f};
    return camera;
  }

  const Vector3 pos = ExtractWorldPosition(tc->worldMatrix);
  const Vector3 forward = ExtractForward(tc->worldMatrix);

  camera.position = pos;
  camera.target = Vector3Add(pos, forward);
  return camera;
}