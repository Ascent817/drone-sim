#pragma once

#include "../physics_world.h"
#include "../world.h"

Entity* CreateGroundPlaneEntity(World& world, PhysicsState* physics,
                                const char* texturePath, float size,
                                float textureTiles);