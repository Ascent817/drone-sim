#pragma once

#include "component.h"
#include "raylib.h"
#include "raymath.h"

class MeshRenderer : public Component {
 public:
  Model model{};
  Matrix normalizeMatrix = MatrixIdentity();
  bool loaded = false;

  MeshRenderer() = default;
  MeshRenderer(const MeshRenderer&) = delete;
  MeshRenderer& operator=(const MeshRenderer&) = delete;
  MeshRenderer(MeshRenderer&&) = default;
  MeshRenderer& operator=(MeshRenderer&&) = default;
  ~MeshRenderer() { if (loaded) UnloadModel(model); }

  void SetTransform(Matrix worldMatrix) {
    model.transform = MatrixMultiply(normalizeMatrix, worldMatrix);
  }

  void Draw(Shader shader) override {
    if (!loaded) return;
    for (int i = 0; i < model.materialCount; i++) {
      model.materials[i].shader = shader;
    }
    DrawModel(model, {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
  }
};