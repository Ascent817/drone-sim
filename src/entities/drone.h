#pragma once

#include "../physics_world.h"
#include "../world.h"

struct DroneEntityResult {
  Entity* drone;
  bool modelLoaded;
};

DroneEntityResult CreateDroneEntity(World& world, PhysicsState* physics,
                                    const char* glbPath, const char* objPath,
                                    float targetSpan, float startY, float mass);