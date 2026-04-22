#include "groundplane.h"

#include "../components/collider.h"
#include "raylib.h"

namespace {
constexpr float kGroundHalfThickness = 0.05f;
}

Entity* CreateGroundPlaneEntity(World& world, PhysicsState* physics,
                                const char* texturePath, float size,
                                float textureTiles) {
  Entity* e = world.CreateEntity();

  auto* gc = e->AddComponent<PlaneCollider>();
  gc->normal = {0.0f, 1.0f, 0.0f};
  gc->distance = 0.0f;

  auto* gr = e->AddComponent<GroundRenderer>();
  gr->texture = LoadTexture(texturePath);
  SetTextureWrap(gr->texture, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(gr->texture, TEXTURE_FILTER_BILINEAR);
  gr->size = size;
  gr->textureTiles = textureTiles;

  auto* groundShape = PhysicsCreateBoxShape(
      physics, rp3d::Vector3(size / 2.0f, kGroundHalfThickness, size / 2.0f));
  PhysicsCreateStaticBody(physics, rp3d::Vector3(0.0f, -kGroundHalfThickness, 0.0f),
                          groundShape, rp3d::Transform::identity());

  return e;
}