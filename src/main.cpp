#include "raylib.h"

static constexpr int kScreenW = 1280;
static constexpr int kScreenH = 720;
static constexpr const char* kModelGlb = "assets/drone.glb";
static constexpr const char* kModelObj = "assets/drone.obj";

static Model TryLoadDroneModel(bool& outLoaded) {
  Model m{};
  if (FileExists(kModelGlb)) {
    m = LoadModel(kModelGlb);
  } else if (FileExists(kModelObj)) {
    m = LoadModel(kModelObj);
  }
  outLoaded = m.meshCount > 0;
  return m;
}

static void DrawPlaceholderDrone() {
  const Vector3 center{0.0f, 1.0f, 0.0f};
  DrawCube(center, 1.0f, 0.2f, 1.0f, RED);
  DrawCubeWires(center, 1.0f, 0.2f, 1.0f, MAROON);

  const float armOffset = 0.55f;
  const float rotorY = 1.15f;
  const Vector3 rotors[4] = {
      {armOffset, rotorY, armOffset},
      {armOffset, rotorY, -armOffset},
      {-armOffset, rotorY, armOffset},
      {-armOffset, rotorY, -armOffset},
  };
  for (const Vector3& p : rotors) {
    DrawCube(p, 0.3f, 0.05f, 0.3f, DARKGRAY);
    DrawCubeWires(p, 0.3f, 0.05f, 0.3f, BLACK);
  }
}

int main() {
  InitWindow(kScreenW, kScreenH, "Drone Sim");
  SetTargetFPS(60);

  Camera3D camera{};
  camera.position = {8.0f, 6.0f, 8.0f};
  camera.target = {0.0f, 1.0f, 0.0f};
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  bool modelLoaded = false;
  Model drone = TryLoadDroneModel(modelLoaded);

  while (!WindowShouldClose()) {
    UpdateCamera(&camera, CAMERA_FREE);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    DrawPlane({0.0f, 0.0f, 0.0f}, {20.0f, 20.0f}, LIGHTGRAY);
    DrawGrid(20, 1.0f);

    if (modelLoaded) {
      DrawModel(drone, {0.0f, 1.0f, 0.0f}, 0.5f, WHITE);
    } else {
      DrawPlaceholderDrone();
    }
    EndMode3D();

    DrawFPS(10, 10);
    if (!modelLoaded) {
      DrawText("Drop drone.glb (or .obj) into assets/ and rebuild to replace the placeholder.",
               10, kScreenH - 30, 18, DARKGRAY);
    }
    EndDrawing();
  }

  if (modelLoaded) UnloadModel(drone);
  CloseWindow();
  return 0;
}
