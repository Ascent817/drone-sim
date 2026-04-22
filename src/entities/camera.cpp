#include "camera.h"

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