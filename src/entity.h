#pragma once

#include <string>
#include <unordered_map>

#include "component.h"
#include "transform.h"

class Entity {
 public:
  virtual void Update(float dt) = 0;
  virtual void Draw() const = 0;

  Transform transform;
  std::unordered_map<std::string, Component*> components;

  unsigned int id = 0;

 private:
  Component components;
};