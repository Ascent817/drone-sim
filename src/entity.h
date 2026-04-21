#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "components/component.h"

class Entity {
 public:
  Entity* AddChild() {
    auto child = std::make_unique<Entity>();
    child->parent = this;
    child->id = static_cast<unsigned int>(children.size());
    Entity* raw = child.get();
    children.push_back(std::move(child));
    return raw;
  }

  Entity* GetParent() const { return parent; }

  template <typename T, typename... Args>
  T* AddComponent(Args&&... args) {
    static_assert(std::is_base_of_v<Component, T>);
    T* raw = new T(std::forward<Args>(args)...);
    components[std::type_index(typeid(T))] =
        std::unique_ptr<Component>(static_cast<Component*>(raw));
    return raw;
  }

  template <typename T>
  T* GetComponent() const {
    auto it = components.find(std::type_index(typeid(T)));
    if (it == components.end()) return nullptr;
    return static_cast<T*>(it->second.get());
  }

  template <typename T>
  bool HasComponent() const {
    return components.count(std::type_index(typeid(T))) > 0;
  }

  template <typename T>
  void RemoveComponent() {
    components.erase(std::type_index(typeid(T)));
  }

  template <typename Fn>
  void ForEachComponent(Fn&& fn) {
    for (auto& [type, comp] : components) fn(*comp);
  }

  template <typename Fn>
  void ForEachChild(Fn&& fn) {
    for (auto& child : children) fn(*child);
  }

  template <typename Fn>
  void ForEachChild(Fn&& fn) const {
    for (const auto& child : children) fn(*child);
  }

  unsigned int id = 0;

 private:
  Entity* parent = nullptr;
  std::vector<std::unique_ptr<Entity>> children;
  std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
};
