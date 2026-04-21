#pragma once

class Component {
 public:
  virtual ~Component() = default;
  unsigned int id = 0;
};