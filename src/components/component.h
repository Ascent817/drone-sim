#pragma once

#include "raylib.h"

class Component {
 public:
  virtual ~Component() = default;
  virtual void Update(float /*dt*/) {}
  virtual void Draw(Shader /*shader*/) {}
  unsigned int id = 0;
};