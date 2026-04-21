#pragma once

#include "physics_world.h"
#include "world.h"

struct DroneSceneResult {
  Entity* drone;
  Entity* camera;
  CameraController* cameraController;
  bool droneModelLoaded;
};

Entity* CreateGroundEntity(World& world, PhysicsState* physics,
                           const char* texturePath, float size,
                           float textureTiles);

DroneSceneResult CreateDroneEntity(World& world, PhysicsState* physics,
                                   const char* glbPath, const char* objPath,
                                   float targetSpan, float startY, float mass);

Entity* CreateCameraEntity(World& world, Vector3 position, Vector3 target);
