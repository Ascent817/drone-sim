# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

3D drone simulation built on raylib 5.5 + ReactPhysics3D 0.10.2 + C++17 with a
custom deferred-style render pipeline (shadow mapping, SSAO, ACES tonemap, gamma,
1.5x SSAA) and an entity-component-system architecture.

## Toolchain (Windows)

- `C:\Program Files\CMake\bin` (CMake 3.25.2) — position 0 on Machine PATH.
- `C:\ProgramData\mingw64\mingw64\bin` (MinGW-w64 GCC 15.2.0) — position 1.
- `C:\MinGW\bin` (legacy GCC 6.3.0) — deprioritized. Do **not** link against it.

**DLL load-order gotcha** in bash/Git-Bash: the old `C:\MinGW\bin` can shadow the
new toolchain's DLLs, causing silent `cc1plus.exe` exit-1 failures. Fix:

```sh
PATH="/c/ProgramData/mingw64/mingw64/bin:$PATH" cmake --build build
```

## Build & run

```sh
cmake -S . -B build -G "MinGW Makefiles"   # first-time / CMakeLists changes
cmake --build build                         # incremental
./build/drone.exe                           # run
```

No tests yet. Smoke check: window opens, ground renders with grid texture, drone
(if model present) drops under gravity with shadows, RMB mouselook + WASD/Space/Shift
camera movement all work, window is resizable.

## Architecture

### Vendored dependencies

Both libraries live under `vendor/` and are not fetched at build time.

- `vendor/raylib/` — pre-built MinGW-w64 static library (`lib/libraylib.a` + headers).
  CMake links it plus `opengl32 gdi32 winmm`. No `raylib.dll` needed at runtime.
- `vendor/reactphysics3d/` — source tree built via `add_subdirectory`. **Include order
  matters:** ReactPhysics3D headers must be included before raylib headers in any
  translation unit that uses both, because raylib's color macros (`RED`, `GREEN`, etc.)
  corrupt RP3D's `DebugColor` enum.

### Entity-Component System

The codebase uses a lightweight ECS where all scene objects (drone, ground, camera)
are entities in a `World::objects` vector.

**Core ECS (`src/entity.h`, `src/component.h`):** `Entity` stores components in an
`unordered_map<type_index, unique_ptr<Component>>` with templated `AddComponent<T>`,
`GetComponent<T>`, `HasComponent<T>`, `RemoveComponent<T>`. One component per type
per entity.

**Components (all header-only):**

| Header | Class | Purpose |
|--------|-------|---------|
| `transform.h` | `TransformComponent` | 4x4 matrix with position/rotation/scale helpers. **Named `TransformComponent` not `Transform`** because raylib defines its own `Transform` struct. |
| `mesh_renderer.h` | `MeshRenderer` | Holds a raylib `Model` + normalize matrix. `Draw(Shader)` assigns the shader to all materials then calls `DrawModel`. |
| `ground_renderer.h` | `GroundRenderer` | rlgl immediate-mode textured quad. `Draw(Shader)` wraps in `BeginShaderMode/EndShaderMode`. |
| `collider.h` | `Collider` (abstract), `BoxCollider`, `MeshCollider`, `PlaneCollider` | Collision shape descriptions. `BoxCollider` used by drone, `PlaneCollider` by ground. |
| `physics_body.h` | `PhysicsBody` | Non-owning pointer to `PhysicsState*`; delegates to `physics_world.h` API. |
| `camera_controller.h` | `CameraController` | Owns `Camera3D` + yaw/pitch state. RMB mouselook, WASD+Space/Shift movement. |

**World (`src/world.h`):** `World::Update()` syncs physics transforms to mesh renderers
and runs camera input. `World::Draw(Shader)` iterates entities and draws ground/mesh
renderers. The `DrawScene` callback in `main.cpp` just calls `world->Draw(shader)`.

### Render pipeline

`src/render_pipeline.{h,cpp}` owns all FBOs, shaders, and per-frame orchestration.
`PipelineRender` runs six passes: (1) shadow depth map 2048x2048; (2) lit geometry
into SSAA G-buffer at 1.5x window size; (3) SSAO from G-buffer depth; (4-5) separable
blur of AO; (6) composite (lit × AO + ACES tonemap + sRGB gamma + sky gradient).

The `DrawScene(Shader, void*)` callback is invoked **twice per frame** — once with
the shadow depth shader, once with the lighting shader. All renderable components
must draw identical geometry in both calls. For models, assign
`materials[i].shader = shader` before `DrawModel`. For immediate-mode geometry,
wrap in `BeginShaderMode(shader)/EndShaderMode()`. The callback must not call
`BeginMode3D` — the pipeline sets up view/projection matrices.

### Physics

`src/physics_world.{h,cpp}` wraps ReactPhysics3D behind an opaque `PhysicsState*`.
Creates a static ground body and a dynamic drone body (box collider). Uses fixed
60 Hz timestep with accumulator. `PhysicsGetDroneTransformMatrix()` returns a raylib
`Matrix` that `World::Update()` feeds into `MeshRenderer::SetTransform()`.

### Asset pipeline

`assets/` at project root is source of truth. A `POST_BUILD` CMake step copies it
to `build/assets/`. Drop models into `assets/`, textures into `assets/textures/`,
shaders into `assets/shaders/`. Never edit `build/assets/` directly.

Drone model loading: checks `assets/drone.glb` then `assets/drone.obj`. If neither
exists, the drone entity simply has no `MeshRenderer` — no placeholder is drawn.
The loaded model is auto-normalized to `kTargetDroneSpan` (2.0) via bounding box.

### Key constraints

- Do not use raylib's built-in camera controls (`UpdateCamera(..., CAMERA_FREE)`).
  `CameraController` is authoritative.
- Fullscreen post-process passes use an explicit fullscreen quad with UVs in `0..1`.
  Do not switch to `DrawRectangle()` without preserving texture coordinates.
- Ground rendering uses rlgl immediate-mode, not a model. Tiling is controlled by
  `GroundRenderer::textureTiles`.
