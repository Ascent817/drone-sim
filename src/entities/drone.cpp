#include "drone.h"

#include <array>
#include <cmath>

#include "../behaviors/rotor.h"
#include "../components/collider.h"
#include "raylib.h"
#include "raymath.h"

namespace {
constexpr float kRotorWidth = 0.35f;
constexpr float kRotorThickness = 0.03f;
constexpr float kRotorHeightOffset = 0.05f;

void CreateDroneRotors(Entity* drone, const Vector3& halfExtents) {
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

    auto* tc = rotor->GetComponent<TransformComponent>();
    tc->setPosition(offset);

    rotor->AddComponent<RotorBehavior>(rotorKeys[i]);

    auto* mr = rotor->AddComponent<MeshRenderer>();
    Mesh rotorMesh = GenMeshCube(kRotorWidth * 0.25f, kRotorThickness, kRotorWidth);
    mr->model = LoadModelFromMesh(rotorMesh);
    mr->loaded = true;

    if (mr->model.materialCount > 0) {
      mr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = DARKGRAY;
    }
  }
}

Matrix ComputeNormalizeMatrix(const Model& model, float targetSpan) {
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
}  // namespace

DroneEntityResult CreateDroneEntity(World& world, PhysicsState* physics,
                                    const char* glbPath, const char* objPath,
                                    float targetSpan, float startY, float mass) {
  Entity* e = world.CreateEntity();

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

  return {e, loaded};
}