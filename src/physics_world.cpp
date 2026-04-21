#include <reactphysics3d/reactphysics3d.h>

#include "physics_world.h"

static constexpr float kPhysicsTimeStep = 1.0f / 60.0f;

struct PhysicsState {
  rp3d::PhysicsCommon common;
  rp3d::PhysicsWorld* world = nullptr;
  float accumulator = 0.0f;
};

Matrix Rp3dTransformToMatrix(const rp3d::Transform& t) {
  float m[16];
  t.getOpenGLMatrix(m);
  return Matrix{
      m[0], m[4], m[8],  m[12],
      m[1], m[5], m[9],  m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15],
  };
}

PhysicsState* PhysicsInit() {
  PhysicsState* ps = new PhysicsState;
  ps->world = ps->common.createPhysicsWorld();
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

rp3d::BoxShape* PhysicsCreateBoxShape(PhysicsState* ps,
                                      const rp3d::Vector3& halfExtents) {
  return ps->common.createBoxShape(halfExtents);
}

rp3d::RigidBody* PhysicsCreateStaticBody(PhysicsState* ps,
                                         const rp3d::Vector3& position,
                                         rp3d::CollisionShape* shape,
                                         const rp3d::Transform& localTransform) {
  rp3d::Transform t(position, rp3d::Quaternion::identity());
  rp3d::RigidBody* body = ps->world->createRigidBody(t);
  body->setType(rp3d::BodyType::STATIC);
  body->addCollider(shape, localTransform);
  return body;
}

rp3d::RigidBody* PhysicsCreateDynamicBody(PhysicsState* ps,
                                          const rp3d::Vector3& position,
                                          rp3d::CollisionShape* shape,
                                          const rp3d::Transform& localTransform,
                                          float mass) {
  rp3d::Transform t(position, rp3d::Quaternion::identity());
  rp3d::RigidBody* body = ps->world->createRigidBody(t);
  body->setType(rp3d::BodyType::DYNAMIC);
  body->addCollider(shape, localTransform);
  body->setMass(mass);
  return body;
}
