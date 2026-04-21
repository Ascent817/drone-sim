#pragma once

#include "component.h"
#include "raymath.h"

class TransformComponent : public Component {
 public:
  // Local transform relative to parent.
  Matrix matrix = MatrixIdentity();
  // Computed world-space transform (updated by World::Update).
  Matrix worldMatrix = MatrixIdentity();

  Vector3 position() const {
    return {matrix.m12, matrix.m13, matrix.m14};
  }

  void setPosition(const Vector3& pos) {
    matrix.m12 = pos.x;
    matrix.m13 = pos.y;
    matrix.m14 = pos.z;
  }

  Vector3 forward() const {
    return {matrix.m8, matrix.m9, matrix.m10};
  }

  Vector3 up() const {
    return {matrix.m4, matrix.m5, matrix.m6};
  }

  Vector3 right() const {
    return {matrix.m0, matrix.m1, matrix.m2};
  }

  Vector3 scale() const {
    return {
        Vector3Length(right()),
        Vector3Length(up()),
        Vector3Length(forward()),
    };
  }

  void setScale(const Vector3& s) {
    Vector3 f = forward();
    Vector3 u = up();
    Vector3 r = right();

    if (Vector3Length(f) > 0.0001f) {
      f = Vector3Scale(Vector3Normalize(f), s.z);
      matrix.m8 = f.x;
      matrix.m9 = f.y;
      matrix.m10 = f.z;
    }

    if (Vector3Length(u) > 0.0001f) {
      u = Vector3Scale(Vector3Normalize(u), s.y);
      matrix.m4 = u.x;
      matrix.m5 = u.y;
      matrix.m6 = u.z;
    }

    if (Vector3Length(r) > 0.0001f) {
      r = Vector3Scale(Vector3Normalize(r), s.x);
      matrix.m0 = r.x;
      matrix.m1 = r.y;
      matrix.m2 = r.z;
    }
  }

  void Translate(const Vector3& delta) {
    matrix.m12 += delta.x;
    matrix.m13 += delta.y;
    matrix.m14 += delta.z;
  }

  void Rotate(float angle, const Vector3& axis) {
    Vector3 axisNorm = Vector3Normalize(axis);
    if (Vector3Length(axisNorm) < 0.0001f) return;

    // Keep local position fixed and rotate only the orientation basis.
    Vector3 p = position();
    Vector3 r = right();
    Vector3 u = up();
    Vector3 f = forward();

    r = Vector3RotateByAxisAngle(r, axisNorm, angle);
    u = Vector3RotateByAxisAngle(u, axisNorm, angle);
    f = Vector3RotateByAxisAngle(f, axisNorm, angle);

    matrix.m0 = r.x;
    matrix.m1 = r.y;
    matrix.m2 = r.z;

    matrix.m4 = u.x;
    matrix.m5 = u.y;
    matrix.m6 = u.z;

    matrix.m8 = f.x;
    matrix.m9 = f.y;
    matrix.m10 = f.z;

    setPosition(p);
  }

  void setRotation(float roll, float pitch, float yaw) {
    Matrix rotX = MatrixRotate({1.0f, 0.0f, 0.0f}, roll);
    Matrix rotY = MatrixRotate({0.0f, 1.0f, 0.0f}, pitch);
    Matrix rotZ = MatrixRotate({0.0f, 0.0f, 1.0f}, yaw);

    // Preserve scale and position
    Vector3 s = scale();
    Vector3 p = position();
    Matrix scaleM = MatrixScale(s.x, s.y, s.z);
    Matrix translateM = MatrixTranslate(p.x, p.y, p.z);

    matrix = MatrixMultiply(translateM, MatrixMultiply(rotZ, MatrixMultiply(rotY, MatrixMultiply(rotX, scaleM))));
  }
};