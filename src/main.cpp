#include <cmath>

#include "raylib.h"
#include "raymath.h"
#include "physics_world.h"
#include "render_pipeline.h"
#include "rlgl.h"
#include "rlights.h"

static constexpr int kScreenW = 1920 / 2;
static constexpr int kScreenH = 1080 / 2;
static constexpr const char* kModelGlb = "assets/drone.glb";
static constexpr const char* kModelObj = "assets/drone.obj";
static constexpr const char* kGroundTexturePath = "assets/textures/grid-dark.png";

struct SceneData {
  Texture2D groundTexture;
  Model drone;
  bool modelLoaded;
  PhysicsState* physics;
  Matrix droneNormalize;
};

static constexpr float kSsaaScale = 1.5f;
static constexpr float kTargetDroneSpan = 2.0f;
static constexpr float kDroneFloorY = 0.75f;
static constexpr float kLookSensitivity = 0.0035f;
static constexpr float kMoveSpeed = 6.0f;
static constexpr float kGroundSize = 100.0f;
static constexpr float kGroundTextureTiles = 10.0f;

struct CameraLookState {
  float yaw;
  float pitch;
  bool rotating;
};

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

static CameraLookState MakeCameraLookState(const Camera3D& camera) {
  Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
  CameraLookState state{};
  state.yaw = atan2f(forward.x, forward.z);
  state.pitch = asinf(forward.y);
  state.rotating = false;
  return state;
}

static void UpdateCameraLook(Camera3D& camera, CameraLookState& state) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    DisableCursor();
    state.rotating = true;
  }
  if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
    EnableCursor();
    state.rotating = false;
  }

  if (!state.rotating) return;

  Vector2 mouseDelta = GetMouseDelta();
  state.yaw -= mouseDelta.x * kLookSensitivity;
  state.pitch -= mouseDelta.y * kLookSensitivity;
  state.pitch = Clamp(state.pitch, -1.5f, 1.5f);

  Vector3 forward = {
      sinf(state.yaw) * cosf(state.pitch),
      sinf(state.pitch),
      cosf(state.yaw) * cosf(state.pitch),
  };
  camera.target = Vector3Add(camera.position, forward);
}

static void UpdateCameraMovement(Camera3D& camera) {
  Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
  Vector3 flatForward = Vector3Normalize(Vector3{forward.x, 0.0f, forward.z});
  if (Vector3Length(flatForward) < 0.0001f) flatForward = Vector3{0.0f, 0.0f, 1.0f};
  Vector3 right = Vector3Normalize(Vector3CrossProduct(flatForward, camera.up));

  Vector3 movement{};
  if (IsKeyDown(KEY_W)) movement = Vector3Add(movement, flatForward);
  if (IsKeyDown(KEY_S)) movement = Vector3Subtract(movement, flatForward);
  if (IsKeyDown(KEY_D)) movement = Vector3Add(movement, right);
  if (IsKeyDown(KEY_A)) movement = Vector3Subtract(movement, right);
  if (IsKeyDown(KEY_SPACE)) movement.y += 1.0f;
  if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) movement.y -= 1.0f;

  if (Vector3Length(movement) < 0.0001f) return;

  movement = Vector3Scale(Vector3Normalize(movement), kMoveSpeed * GetFrameTime());
  camera.position = Vector3Add(camera.position, movement);
  camera.target = Vector3Add(camera.target, movement);
}

static Matrix ComputeDroneNormalizeMatrix(const Model& model) {
  BoundingBox bounds = GetModelBoundingBox(model);
  Vector3 size = Vector3Subtract(bounds.max, bounds.min);
  float largestDim = fmaxf(size.x, fmaxf(size.y, size.z));
  if (largestDim <= 0.0001f) return MatrixIdentity();

  float scale = kTargetDroneSpan / largestDim;
  Vector3 center = {
      (bounds.min.x + bounds.max.x) * 0.5f,
      (bounds.min.y + bounds.max.y) * 0.5f,
      (bounds.min.z + bounds.max.z) * 0.5f,
  };
  Vector3 translation = {
      -center.x * scale,
      -bounds.min.y * scale,
      -center.z * scale,
  };

  Matrix scaleM = MatrixScale(scale, scale, scale);
  Matrix translateM = MatrixTranslate(translation.x, translation.y, translation.z);
  return MatrixMultiply(scaleM, translateM);
}

static void DrawPlaceholderDrone(Vector3 floorPos) {
  const Vector3 center{floorPos.x, floorPos.y + 0.1f, floorPos.z};
  DrawCube(center, 1.0f, 0.2f, 1.0f, RED);
  DrawCubeWires(center, 1.0f, 0.2f, 1.0f, MAROON);

  const float armOffset = 0.55f;
  const float rotorY = floorPos.y + 0.25f;
  const Vector3 rotors[4] = {
      {floorPos.x + armOffset, rotorY, floorPos.z + armOffset},
      {floorPos.x + armOffset, rotorY, floorPos.z - armOffset},
      {floorPos.x - armOffset, rotorY, floorPos.z + armOffset},
      {floorPos.x - armOffset, rotorY, floorPos.z - armOffset},
  };
  for (const Vector3& p : rotors) {
    DrawCube(p, 0.3f, 0.05f, 0.3f, DARKGRAY);
    DrawCubeWires(p, 0.3f, 0.05f, 0.3f, BLACK);
  }
}

static void DrawGround(Texture2D texture) {
  const float halfSize = kGroundSize * 0.5f;

  rlSetTexture(texture.id);
  rlBegin(RL_QUADS);
  rlColor4ub(255, 255, 255, 255);

  rlNormal3f(0.0f, 1.0f, 0.0f);
  rlTexCoord2f(0.0f, 0.0f);
  rlVertex3f(-halfSize, 0.0f, -halfSize);

  rlNormal3f(0.0f, 1.0f, 0.0f);
  rlTexCoord2f(0.0f, kGroundTextureTiles);
  rlVertex3f(-halfSize, 0.0f, halfSize);

  rlNormal3f(0.0f, 1.0f, 0.0f);
  rlTexCoord2f(kGroundTextureTiles, kGroundTextureTiles);
  rlVertex3f(halfSize, 0.0f, halfSize);

  rlNormal3f(0.0f, 1.0f, 0.0f);
  rlTexCoord2f(kGroundTextureTiles, 0.0f);
  rlVertex3f(halfSize, 0.0f, -halfSize);

  rlEnd();
  rlSetTexture(0);
}

static void DrawScene(Shader shader, void* user) {
  SceneData* sd = static_cast<SceneData*>(user);

  BeginShaderMode(shader);
  DrawGround(sd->groundTexture);
  EndShaderMode();

  if (sd->modelLoaded) {
    Matrix physMat = PhysicsGetDroneTransformMatrix(sd->physics);
    sd->drone.transform = MatrixMultiply(sd->droneNormalize, physMat);
    for (int i = 0; i < sd->drone.materialCount; i++) {
      sd->drone.materials[i].shader = shader;
    }
    DrawModel(sd->drone, {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
  } else {
    BeginShaderMode(shader);
    Vector3 dronePos = PhysicsGetDronePosition(sd->physics);
    DrawPlaceholderDrone(dronePos);
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
  CameraLookState lookState = MakeCameraLookState(camera);

  int renderW = GetRenderWidth();
  int renderH = GetRenderHeight();
  Pipeline pipeline = PipelineCreate(renderW, renderH, kSsaaScale);

  Light lights[3];
  ConfigureLights(lights, pipeline.lighting);
  PipelineSetLights(pipeline, lights, 3, 0);

  SceneData scene{};
  scene.groundTexture = LoadTexture(kGroundTexturePath);
  SetTextureWrap(scene.groundTexture, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(scene.groundTexture, TEXTURE_FILTER_BILINEAR);
  scene.drone = TryLoadDroneModel(scene.modelLoaded);
  Vector3 droneHalfExtents = {0.5f, 0.175f, 0.5f};
  if (scene.modelLoaded) {
    scene.droneNormalize = ComputeDroneNormalizeMatrix(scene.drone);
    BoundingBox rawBounds = GetModelBoundingBox(scene.drone);
    Vector3 rawSize = Vector3Subtract(rawBounds.max, rawBounds.min);
    float largest = fmaxf(rawSize.x, fmaxf(rawSize.y, rawSize.z));
    float s = kTargetDroneSpan / largest;
    droneHalfExtents = {rawSize.x * s * 0.5f, rawSize.y * s * 0.5f, rawSize.z * s * 0.5f};
  } else {
    scene.droneNormalize = MatrixIdentity();
  }

  scene.physics = PhysicsInit(kGroundSize, kDroneFloorY, droneHalfExtents, 1.0f);

  while (!WindowShouldClose()) {
    UpdateCameraMovement(camera);
    UpdateCameraLook(camera, lookState);
    PhysicsStep(scene.physics, GetFrameTime());

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

  PhysicsDestroy(scene.physics);
  UnloadTexture(scene.groundTexture);
  if (scene.modelLoaded) UnloadModel(scene.drone);
  if (lookState.rotating) EnableCursor();
  PipelineDestroy(pipeline);
  CloseWindow();
  return 0;
}
