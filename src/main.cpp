#include <cmath>

#include "raylib.h"
#include "raymath.h"
#include "render_pipeline.h"
#include "rlights.h"

static constexpr int kScreenW = 1920 / 2;
static constexpr int kScreenH = 1080 / 2;
static constexpr const char* kModelGlb = "assets/drone.glb";
static constexpr const char* kModelObj = "assets/drone.obj";

struct SceneData {
  Model drone;
  bool modelLoaded;
};

static constexpr float kSsaaScale = 1.5f;
static constexpr float kTargetDroneSpan = 2.0f;
static constexpr float kDroneFloorY = 0.75f;

static void ConfigureLights(Light (&lights)[3], Shader shader) {
  lights[0] = CreateLight(0, LIGHT_DIRECTIONAL, {6.0f, 12.0f, 6.0f}, {0.0f, 0.0f, 0.0f},
                          Color{255, 240, 210, 255}, shader);
  lights[1] = CreateLight(1, LIGHT_DIRECTIONAL, {-6.0f, 4.0f, -3.0f}, {0.0f, 0.0f, 0.0f},
                          Color{90, 110, 140, 255}, shader);
  lights[2] = CreateLight(2, LIGHT_DIRECTIONAL, {0.0f, 5.0f, -10.0f}, {0.0f, 0.0f, 0.0f},
                          Color{200, 220, 255, 255}, shader);
}

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

static void NormalizeDroneModel(Model& model) {
  BoundingBox bounds = GetModelBoundingBox(model);
  Vector3 size = Vector3Subtract(bounds.max, bounds.min);
  float largestDim = fmaxf(size.x, fmaxf(size.y, size.z));
  if (largestDim <= 0.0001f) return;

  float scale = kTargetDroneSpan / largestDim;
  Vector3 center = {
      (bounds.min.x + bounds.max.x) * 0.5f,
      (bounds.min.y + bounds.max.y) * 0.5f,
      (bounds.min.z + bounds.max.z) * 0.5f,
  };
  Vector3 translation = {
      -center.x * scale,
      kDroneFloorY - bounds.min.y * scale,
      -center.z * scale,
  };

  Matrix scaleM = MatrixScale(scale, scale, scale);
  Matrix translateM = MatrixTranslate(translation.x, translation.y, translation.z);
  model.transform = MatrixMultiply(scaleM, model.transform);
  model.transform = MatrixMultiply(translateM, model.transform);
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

static void DrawScene(Shader shader, void* user) {
  SceneData* sd = static_cast<SceneData*>(user);

  BeginShaderMode(shader);
  DrawPlane({0.0f, 0.0f, 0.0f}, {20.0f, 20.0f}, LIGHTGRAY);
  EndShaderMode();

  if (sd->modelLoaded) {
    for (int i = 0; i < sd->drone.materialCount; i++) {
      sd->drone.materials[i].shader = shader;
    }
    DrawModel(sd->drone, {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
  } else {
    BeginShaderMode(shader);
    DrawPlaceholderDrone();
    EndShaderMode();
  }
}

int main() {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(kScreenW, kScreenH, "Drone Sim");
  SetTargetFPS(60);

  Camera3D camera{};
  camera.position = {8.0f, 6.0f, 8.0f};
  camera.target = {0.0f, 1.0f, 0.0f};
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  int renderW = GetRenderWidth();
  int renderH = GetRenderHeight();
  Pipeline pipeline = PipelineCreate(renderW, renderH, kSsaaScale);

  Light lights[3];
  ConfigureLights(lights, pipeline.lighting);
  PipelineSetLights(pipeline, lights, 3, 0);

  SceneData scene{};
  scene.drone = TryLoadDroneModel(scene.modelLoaded);
  if (scene.modelLoaded) NormalizeDroneModel(scene.drone);

  while (!WindowShouldClose()) {
    UpdateCamera(&camera, CAMERA_FREE);

    if (renderW != GetRenderWidth() || renderH != GetRenderHeight()) {
      renderW = GetRenderWidth();
      renderH = GetRenderHeight();
      PipelineDestroy(pipeline);
      pipeline = PipelineCreate(renderW, renderH, kSsaaScale);
      ConfigureLights(lights, pipeline.lighting);
      PipelineSetLights(pipeline, lights, 3, 0);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    PipelineRender(pipeline, camera, DrawScene, &scene);

    DrawFPS(10, 10);
    if (!scene.modelLoaded) {
      DrawText("Drop drone.glb (or .obj) into assets/ and rebuild to replace the placeholder.",
               10, GetScreenHeight() - 30, 18, DARKGRAY);
    }
    EndDrawing();
  }

  if (scene.modelLoaded) UnloadModel(scene.drone);
  PipelineDestroy(pipeline);
  CloseWindow();
  return 0;
}
