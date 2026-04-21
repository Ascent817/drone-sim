#pragma once

#include <memory>
#include <vector>

#include "physics_body.h"
#include "camera_controller.h"
#include "entity.h"
#include "ground_renderer.h"
#include "mesh_renderer.h"
#include "transform.h"

struct World {
  std::vector<std::unique_ptr<Entity>> objects;

  Entity* CreateEntity() {
    auto e = std::make_unique<Entity>();
    e->id = static_cast<unsigned int>(objects.size());
    Entity* raw = e.get();
    objects.push_back(std::move(e));
    return raw;
  }

  void Update(float dt) {
    // System passes: cross-component data flow
    for (auto& e : objects) {
      if (auto* pb = e->GetComponent<PhysicsBody>()) {
        if (auto* tc = e->GetComponent<TransformComponent>()) {
          tc->matrix = pb->GetTransformMatrix();
        }
      }
      if (auto* mr = e->GetComponent<MeshRenderer>()) {
        if (auto* tc = e->GetComponent<TransformComponent>()) {
          mr->SetTransform(tc->matrix);
        }
      }
    }
    // Generic component update
    for (auto& e : objects) {
      e->ForEachComponent([dt](Component& c) { c.Update(dt); });
    }
  }

  void Draw(Shader shader) {
    for (auto& e : objects) {
      e->ForEachComponent([shader](Component& c) { c.Draw(shader); });
    }
  }
};
