#pragma once

#include "component.h"
#include "raylib.h"
#include "rlgl.h"

class GroundRenderer : public Component {
 public:
  Texture2D texture{};
  float size = 100.0f;
  float textureTiles = 10.0f;

  GroundRenderer() = default;
  GroundRenderer(const GroundRenderer&) = delete;
  GroundRenderer& operator=(const GroundRenderer&) = delete;
  GroundRenderer(GroundRenderer&&) = default;
  GroundRenderer& operator=(GroundRenderer&&) = default;
  ~GroundRenderer() { if (texture.id != 0) UnloadTexture(texture); }

  void Draw(Shader shader) override {
    float halfSize = size * 0.5f;

    BeginShaderMode(shader);
    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);

    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex3f(-halfSize, 0.0f, -halfSize);

    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(0.0f, textureTiles);
    rlVertex3f(-halfSize, 0.0f, halfSize);

    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(textureTiles, textureTiles);
    rlVertex3f(halfSize, 0.0f, halfSize);

    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(textureTiles, 0.0f);
    rlVertex3f(halfSize, 0.0f, -halfSize);

    rlEnd();
    rlSetTexture(0);
    EndShaderMode();
  }
};