#include <reactphysics3d/reactphysics3d.h>

#include "physics_world.h"

static constexpr float kPhysicsTimeStep = 1.0f / 60.0f;

struct PhysicsState {
  rp3d::PhysicsCommon common;
  rp3d::PhysicsWorld* world = nullptr;

  rp3d::RigidBody* groundBody = nullptr;
  rp3d::BoxShape* groundShape = nullptr;

  rp3d::RigidBody* droneBody = nullptr;
  rp3d::BoxShape* droneShape = nullptr;

  float accumulator = 0.0f;
};

static Matrix Rp3dTransformToMatrix(const rp3d::Transform& t) {
  float m[16];
  t.getOpenGLMatrix(m);
  return Matrix{
      m[0],
      m[4],
      m[8],
      m[12],
      m[1],
      m[5],
      m[9],
      m[13],
      m[2],
      m[6],
      m[10],
      m[14],
      m[3],
      m[7],
      m[11],
      m[15],
  };
}

static Vector3 Rp3dToRaylib(const rp3d::Vector3& v) {
  return Vector3{v.x, v.y, v.z};
}

PhysicsState* PhysicsInit(float groundSize, float droneStartY,
                          Vector3 droneHalfExtents, float droneMass) {
  PhysicsState* ps = new PhysicsState;

  ps->world = ps->common.createPhysicsWorld();

  float groundHalfThickness = 0.05f;
  ps->groundShape = ps->common.createBoxShape(
      rp3d::Vector3(groundSize / 2.0f, groundHalfThickness, groundSize / 2.0f));

  rp3d::Transform groundTransform(
      rp3d::Vector3(0.0f, -groundHalfThickness, 0.0f),
      rp3d::Quaternion::identity());

  ps->groundBody = ps->world->createRigidBody(groundTransform);
  ps->groundBody->setType(rp3d::BodyType::STATIC);
  ps->groundBody->addCollider(ps->groundShape, rp3d::Transform::identity());

  ps->droneShape = ps->common.createBoxShape(
      rp3d::Vector3(droneHalfExtents.x, droneHalfExtents.y, droneHalfExtents.z));

  rp3d::Transform droneTransform(
      rp3d::Vector3(0.0f, droneStartY, 0.0f),
      rp3d::Quaternion::identity());

  ps->droneBody = ps->world->createRigidBody(droneTransform);
  ps->droneBody->setType(rp3d::BodyType::DYNAMIC);

  rp3d::Transform colliderLocal(
      rp3d::Vector3(0.0f, droneHalfExtents.y, 0.0f),
      rp3d::Quaternion::identity());
  ps->droneBody->addCollider(ps->droneShape, colliderLocal);
  ps->droneBody->setMass(droneMass);

  ps->accumulator = 0.0f;
  return ps;
}

void PhysicsDestroy(PhysicsState* ps) {
  if (ps) {
    if (ps->world) {
      ps->common.destroyPhysicsWorld(ps->world);
      ps->world = nullptr;
    }
    delete ps;
  }
}

void PhysicsStep(PhysicsState* ps, float dt) {
  ps->accumulator += dt;
  if (ps->accumulator > kPhysicsTimeStep * 10.0f)
    ps->accumulator = kPhysicsTimeStep * 10.0f;
  while (ps->accumulator >= kPhysicsTimeStep) {
    ps->world->update(kPhysicsTimeStep);
    ps->accumulator -= kPhysicsTimeStep;
  }
}

Vector3 PhysicsGetDronePosition(const PhysicsState* ps) {
  return Rp3dToRaylib(ps->droneBody->getTransform().getPosition());
}

Matrix PhysicsGetDroneTransformMatrix(const PhysicsState* ps) {
  return Rp3dTransformToMatrix(ps->droneBody->getTransform());
}
