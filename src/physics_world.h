#pragma once

#include <reactphysics3d/reactphysics3d.h>

#include "raylib.h"
#include "raymath.h"

struct PhysicsState;

Matrix Rp3dTransformToMatrix(const rp3d::Transform& t);

PhysicsState* PhysicsInit();
void PhysicsDestroy(PhysicsState* ps);
void PhysicsStep(PhysicsState* ps, float dt);

rp3d::BoxShape* PhysicsCreateBoxShape(PhysicsState* ps,
                                      const rp3d::Vector3& halfExtents);

rp3d::RigidBody* PhysicsCreateStaticBody(PhysicsState* ps,
                                         const rp3d::Vector3& position,
                                         rp3d::CollisionShape* shape,
                                         const rp3d::Transform& localTransform);

rp3d::RigidBody* PhysicsCreateDynamicBody(PhysicsState* ps,
                                          const rp3d::Vector3& position,
                                          rp3d::CollisionShape* shape,
                                          const rp3d::Transform& localTransform,
                                          float mass);
