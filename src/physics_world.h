#pragma once

#include "raylib.h"
#include "raymath.h"

struct PhysicsState;

PhysicsState* PhysicsInit(float groundSize, float droneStartY,
                          Vector3 droneHalfExtents, float droneMass);

void PhysicsDestroy(PhysicsState* ps);

void PhysicsStep(PhysicsState* ps, float dt);

Vector3 PhysicsGetDronePosition(const PhysicsState* ps);

Matrix PhysicsGetDroneTransformMatrix(const PhysicsState* ps);
