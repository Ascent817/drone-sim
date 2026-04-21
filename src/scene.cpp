#include "scene.h"

#include <array>
#include <cmath>

#include "behaviors/rotor.h"
#include "components/collider.h"
#include "physics_world.h"
#include "raylib.h"
#include "raymath.h"

static constexpr float kGroundHalfThickness = 0.05f;
static constexpr float kRotorWidth = 0.35f;
static constexpr float kRotorThickness = 0.03f;
static constexpr float kRotorHeightOffset = 0.05f;

static void CreateDroneRotors(Entity* drone, const Vector3& halfExtents) {
  const float x = halfExtents.x * 0.73f;
  const float y = halfExtents.y * 1.0f + kRotorHeightOffset;
  const float z = halfExtents.z * 0.73f;

  const std::array<Vector3, 4> rotorOffsets = {
      Vector3{x, y, z},
      Vector3{-x, y, z},
      Vector3{x, y, -z},
      Vector3{-x, y, -z},
  };

  const std::array<int, 4> rotorKeys = {
      KEY_ONE,
      KEY_TWO,
      KEY_THREE,
      KEY_FOUR,
  };

  for (size_t i = 0; i < rotorOffsets.size(); i++) {
    const Vector3& offset = rotorOffsets[i];
    Entity* rotor = drone->AddChild();

    auto* tc = rotor->AddComponent<TransformComponent>();
    tc->setPosition(offset);

    rotor->AddComponent<RotorBehavior>(tc, rotorKeys[i]);

    auto* mr = rotor->AddComponent<MeshRenderer>();
    Mesh rotorMesh = GenMeshCube(kRotorWidth * 0.25f, kRotorThickness, kRotorWidth);
    mr->model = LoadModelFromMesh(rotorMesh);
    mr->loaded = true;

    if (mr->model.materialCount > 0) {
      mr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = DARKGRAY;
    }
  }
}

static Matrix ComputeNormalizeMatrix(const Model& model, float targetSpan) {
  BoundingBox bounds = GetModelBoundingBox(model);
  Vector3 size = Vector3Subtract(bounds.max, bounds.min);
  float largestDim = fmaxf(size.x, fmaxf(size.y, size.z));
  if (largestDim <= 0.0001f) return MatrixIdentity();

  float scale = targetSpan / largestDim;
  Vector3 translation = {
      -(bounds.min.x + bounds.max.x) * 0.5f * scale,
      -bounds.min.y * scale,
      -(bounds.min.z + bounds.max.z) * 0.5f * scale,
  };
  return MatrixMultiply(MatrixScale(scale, scale, scale),
                        MatrixTranslate(translation.x, translation.y, translation.z));
}

Entity* CreateGroundEntity(World& world, PhysicsState* physics,
                           const char* texturePath, float size,
                           float textureTiles) {
  Entity* e = world.CreateEntity();
  e->AddComponent<TransformComponent>();

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

DroneSceneResult CreateDroneEntity(World& world, PhysicsState* physics,
                                   const char* glbPath, const char* objPath,
                                   float targetSpan, float startY, float mass) {
  Entity* e = world.CreateEntity();
  e->AddComponent<TransformComponent>();

  Vector3 droneHalfExtents = {0.5f, 0.175f, 0.5f};
  bool loaded = false;

  Model model{};
  if (FileExists(glbPath)) {
    model = LoadModel(glbPath);
  } else if (FileExists(objPath)) {
    model = LoadModel(objPath);
  }

  if (model.meshCount > 0) {
    BoundingBox rawBounds = GetModelBoundingBox(model);
    Vector3 rawSize = Vector3Subtract(rawBounds.max, rawBounds.min);
    float largest = fmaxf(rawSize.x, fmaxf(rawSize.y, rawSize.z));
    float s = targetSpan / largest;
    droneHalfExtents = {rawSize.x * s * 0.5f, rawSize.y * s * 0.5f, rawSize.z * s * 0.5f};

    auto* mr = e->AddComponent<MeshRenderer>();
    mr->model = model;
    mr->normalizeMatrix = ComputeNormalizeMatrix(model, targetSpan);
    mr->loaded = true;
    loaded = true;
  }

  auto* dc = e->AddComponent<BoxCollider>();
  dc->halfExtents = droneHalfExtents;
  dc->offset = {0.0f, droneHalfExtents.y, 0.0f};

  auto* droneShape = PhysicsCreateBoxShape(
      physics, rp3d::Vector3(droneHalfExtents.x, droneHalfExtents.y, droneHalfExtents.z));
  rp3d::Transform colliderLocal(rp3d::Vector3(0.0f, droneHalfExtents.y, 0.0f),
                                rp3d::Quaternion::identity());
  rp3d::RigidBody* droneBody = PhysicsCreateDynamicBody(
      physics, rp3d::Vector3(0.0f, startY, 0.0f), droneShape, colliderLocal, mass);

  auto* pb = e->AddComponent<PhysicsBody>();
  pb->body = droneBody;

  CreateDroneRotors(e, droneHalfExtents);

  return {e, nullptr, nullptr, loaded};
}

Entity* CreateCameraEntity(World& world, Vector3 position, Vector3 target) {
  Entity* e = world.CreateEntity();
  e->AddComponent<TransformComponent>();

  auto* cc = e->AddComponent<CameraController>();
  cc->camera.position = position;
  cc->camera.target = target;
  cc->camera.up = {0.0f, 1.0f, 0.0f};
  cc->camera.fovy = 45.0f;
  cc->camera.projection = CAMERA_PERSPECTIVE;
  cc->InitFromCamera();

  return e;
}
