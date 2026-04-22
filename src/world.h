#pragma once

#include <memory>
#include <vector>

#include "components/physics_body.h"
#include "components/camera_controller.h"
#include "components/ground_renderer.h"
#include "components/mesh_renderer.h"
#include "components/transform.h"
#include "entity.h"

struct World {
  std::vector<std::unique_ptr<Entity>> objects;

  static void StartComponentsRecursive(Entity& entity) {
    entity.ForEachComponent([](Component& c) {
      if (c.started) return;
      c.Start();
      c.started = true;
    });
    entity.ForEachChild([](Entity& child) { StartComponentsRecursive(child); });
  }

  static void SyncPhysicsToTransform(Entity& entity) {
    if (auto* pb = entity.GetComponent<PhysicsBody>()) {
      if (auto* tc = entity.GetComponent<TransformComponent>()) {
        tc->matrix = pb->GetTransformMatrix();
      }
    }

    entity.ForEachChild([](Entity& child) { SyncPhysicsToTransform(child); });
  }

  static void ComputeWorldTransforms(Entity& entity, const Matrix& parentWorld) {
    Matrix currentWorld = parentWorld;

    if (auto* tc = entity.GetComponent<TransformComponent>()) {
      // Compose local with parent in the same order used by render transforms.
      tc->worldMatrix = MatrixMultiply(tc->matrix, parentWorld);
      currentWorld = tc->worldMatrix;
    }

    entity.ForEachChild([&currentWorld](Entity& child) {
      ComputeWorldTransforms(child, currentWorld);
    });
  }

  static void SyncMeshFromTransform(Entity& entity) {
    if (auto* mr = entity.GetComponent<MeshRenderer>()) {
      if (auto* tc = entity.GetComponent<TransformComponent>()) {
        mr->SetTransform(tc->worldMatrix);
      }
    }

    entity.ForEachChild([](Entity& child) { SyncMeshFromTransform(child); });
  }

  static void UpdateComponentsRecursive(Entity& entity, float dt) {
    entity.ForEachComponent([dt](Component& c) { c.Update(dt); });
    entity.ForEachChild([dt](Entity& child) { UpdateComponentsRecursive(child, dt); });
  }

  static void DrawRecursive(Entity& entity, Shader shader) {
    entity.ForEachComponent([shader](Component& c) { c.Draw(shader); });
    entity.ForEachChild([shader](Entity& child) { DrawRecursive(child, shader); });
  }

  Entity* CreateEntity() {
    auto e = std::make_unique<Entity>();
    e->id = static_cast<unsigned int>(objects.size());
    Entity* raw = e.get();
    objects.push_back(std::move(e));
    return raw;
  }

  void Update(float dt) {
    for (auto& e : objects) {
      StartComponentsRecursive(*e);
    }

    for (auto& e : objects) {
      SyncPhysicsToTransform(*e);
    }

    for (auto& e : objects) {
      UpdateComponentsRecursive(*e, dt);
    }

    for (auto& e : objects) {
      ComputeWorldTransforms(*e, MatrixIdentity());
    }

    for (auto& e : objects) {
      SyncMeshFromTransform(*e);
    }
  }

  void Draw(Shader shader) {
    for (auto& e : objects) {
      DrawRecursive(*e, shader);
    }
  }
};
