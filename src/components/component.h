#pragma once

class Entity;

#include "raylib.h"

class Component {
 public:
  virtual ~Component() = default;
  virtual void Start() {}
  virtual void Update(float /*dt*/) {}
  virtual void Draw(Shader /*shader*/) {}

  Entity* GetEntity() const { return entity; }

  bool started = false;
  unsigned int id = 0;

 protected:
  Entity* entity = nullptr;

 private:
  friend class Entity;
};