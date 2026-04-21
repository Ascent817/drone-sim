#include "physics_world.h"
#include "scene.h"

#include "raylib.h"
#include "render_pipeline.h"
#include "rlights.h"
#include "world.h"

static constexpr int kScreenW = 1920 / 2;
static constexpr int kScreenH = 1080 / 2;
static constexpr const char* kModelGlb = "assets/drone.glb";
static constexpr const char* kModelObj = "assets/drone.obj";
static constexpr const char* kGroundTexturePath = "assets/textures/grid-dark.png";

static constexpr float kSsaaScale = 1.5f;
static constexpr float kTargetDroneSpan = 2.0f;
static constexpr float kDroneFloorY = 0.75f;
static constexpr float kGroundSize = 100.0f;
static constexpr float kGroundTextureTiles = 10.0f;

static void ConfigureLights(Light (&lights)[3], Shader shader) {
  lights[0] = CreateLight(0, LIGHT_DIRECTIONAL, {6.0f, 12.0f, 6.0f}, {0.0f, 0.0f, 0.0f},
                          Color{255, 240, 210, 255}, shader);
  lights[1] = CreateLight(1, LIGHT_DIRECTIONAL, {-6.0f, 4.0f, -3.0f}, {0.0f, 0.0f, 0.0f},
                          Color{90, 110, 140, 255}, shader);
  lights[2] = CreateLight(2, LIGHT_DIRECTIONAL, {0.0f, 5.0f, -10.0f}, {0.0f, 0.0f, 0.0f},
                          Color{200, 220, 255, 255}, shader);
}

static void DrawScene(Shader shader, void* user) {
  static_cast<World*>(user)->Draw(shader);
}

int main() {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
  InitWindow(kScreenW, kScreenH, "Drone Sim");
  SetTargetFPS(200);

  int renderW = GetRenderWidth();
  int renderH = GetRenderHeight();
  Pipeline pipeline = PipelineCreate(renderW, renderH, kSsaaScale);

  Light lights[3];
  ConfigureLights(lights, pipeline.lighting);
  PipelineSetLights(pipeline, lights, 3, 0);

  PhysicsState* physics = PhysicsInit();
  World world;

  CreateGroundEntity(world, physics, kGroundTexturePath, kGroundSize, kGroundTextureTiles);
  DroneSceneResult drone = CreateDroneEntity(world, physics, kModelGlb, kModelObj,
                                             kTargetDroneSpan, kDroneFloorY, 1.0f);
  Entity* cameraEntity = CreateCameraEntity(world, {8.0f, 6.0f, 8.0f}, {0.0f, 1.0f, 0.0f});
  auto* cc = cameraEntity->GetComponent<CameraController>();

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    PhysicsStep(physics, dt);
    world.Update(dt);

    if (renderW != GetRenderWidth() || renderH != GetRenderHeight()) {
      renderW = GetRenderWidth();
      renderH = GetRenderHeight();
      PipelineResize(pipeline, renderW, renderH, kSsaaScale);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    PipelineRender(pipeline, cc->camera, DrawScene, &world);

    DrawFPS(10, 10);
    if (!drone.droneModelLoaded) {
      DrawText("Drop drone.glb (or .obj) into assets/ and rebuild.",
               10, GetScreenHeight() - 30, 18, DARKGRAY);
    }
    EndDrawing();
  }

  PhysicsDestroy(physics);
  world.objects.clear();
  PipelineDestroy(pipeline);
  CloseWindow();
  return 0;
}
