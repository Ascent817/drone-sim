#pragma once

#include "../world.h"

Entity* CreateCameraEntity(World& world, Vector3 position, Vector3 target);

Entity* CreateDroneCameraEntity(Entity* drone, Vector3 localPosition);

Camera3D BuildCameraFromEntityTransform(const Entity* cameraEntity,
                                        float fovy = 60.0f);