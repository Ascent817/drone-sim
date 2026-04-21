# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

3D drone simulation built on raylib + C++17 with a custom deferred-style render
pipeline (shadow mapping, SSAO, ACES tonemap, gamma, 1.5x SSAA). `src/main.cpp`
currently wires the scene, textured ground, custom camera/input controls, and the
drone-with-placeholder fallback. `src/render_pipeline.{h,cpp}` owns the FBOs,
shaders, and per-frame orchestration. `src/rlights.h` is the light struct +
uniform upload helper (adapted from raylib's `examples/shaders/rlights.h` since
the vendored zip ships headers-only). Subsequent work (ReactPhysics3D
integration, drone forces/controls, telemetry) builds on top of this skeleton.

## Toolchain (Windows)

The Machine PATH has been ordered so the right tools win in any fresh shell:

- `C:\Program Files\CMake\bin` (CMake 3.25.2) - position 0 on Machine PATH.
- `C:\ProgramData\mingw64\mingw64\bin` (MinGW-w64 GCC 15.2.0, `choco install mingw`) - position 1.
- `C:\MinGW\bin` (legacy GCC 6.3.0 from 2016) - still present but deprioritized. Do **not**
  link the pre-built raylib against this old toolchain; it produces libgcc/libstdc++
  ABI errors. If a build picks it up, fix PATH order rather than working around it.

Already-running shells (including bash sessions inherited from the old environment)
won't see the change until restart. New shells are fine.

**DLL load-order gotcha** (important in bash/Git-Bash shells): Git Bash's inherited
PATH keeps `C:\MinGW\bin` ahead of the new toolchain, so when `g++.exe` spawns
`cc1plus.exe` the Windows loader pulls the old `libstdc++-6.dll`, `libgcc_s_*.dll`,
`libmpc-3.dll`, etc. from there first. `cc1plus.exe` then dies silently with exit 1
and zero stderr output. Fix by prepending the new toolchain's `bin/` in the shell
that's invoking the build:

```sh
PATH="/c/ProgramData/mingw64/mingw64/bin:$PATH" cmake --build build
```

This is only needed inside bash shells started from the old environment. A fresh
cmd/PowerShell (or a new bash started after the Machine PATH was fixed) picks up
the right DLLs automatically.

## Build & run

First-time configure (and any time `CMakeLists.txt` changes):
```sh
cmake -S . -B build -G "MinGW Makefiles"
```

Incremental build + run:
```sh
cmake --build build
./build/drone.exe
```

There are no tests yet. The smoke check is: launch `drone.exe`, confirm a
resizable HiDPI-aware window opens, a lit drone (or placeholder) casts a shadow
onto a large textured grid floor under a horizon-gradient sky, mouselook only
engages while the right mouse button is held, `WASD` moves horizontally,
`Space` moves up, `Shift` moves down, and the process stays alive.

## Architecture

**Vendored raylib, not fetched.** `vendor/raylib/` contains the official pre-built
MinGW-w64 5.5 release (`include/`, `lib/libraylib.a`). CMake links the **static**
`libraylib.a` plus the Windows system libs raylib needs (`opengl32 gdi32 winmm`),
so `drone.exe` runs without shipping `raylib.dll`. There is no `find_package` /
`FetchContent` - pointing CMake at `vendor/raylib` keeps the build hermetic and
offline. If you upgrade raylib, replace the contents of `vendor/raylib/` in place.

**Asset pipeline.** `assets/` at the project root is the source of truth. A
`POST_BUILD` step in `CMakeLists.txt` copies the whole directory next to the built
executable (`build/assets/`), so `LoadModel("assets/drone.glb")` resolves whether the
exe is launched from the project root or from `build/`. Don't edit `build/assets/`
- it's regenerated every build. Drop new model files into `assets/`, textures into
`assets/textures/`, shaders into `assets/shaders/`, and rebuild.

**Model loading is fallback-driven.** `TryLoadDroneModel` in `src/main.cpp` checks
for `assets/drone.glb` first, then `assets/drone.obj`, and reports success via
`model.meshCount > 0` (raylib's documented signal - a failed `LoadModel` returns a
zero-mesh `Model`, not an error code). When no model loads, `DrawPlaceholderDrone`
renders a cube body + four rotor cubes so the scene is always usable. Any model
work should preserve this fallback so a missing asset never crashes the app. The
loaded drone model is normalized after load using `GetModelBoundingBox()` so the
initial framing does not depend on authoring scale/origin inside the source GLB.

**Render pipeline.** `PipelineRender` runs six passes per frame against custom
FBOs built via `rlgl.h`: (1) depth-only shadow map from the sun light into a
2048^2 depth texture; (2) main lit geometry with 3-point Phong + 3x3 PCF shadow
sampling into a SSAA G-buffer (color + sampleable depth) at `windowSize * 1.5`;
(3) Crytek-style SSAO reading the G-buffer depth with normals reconstructed via
`dFdx/dFdy`; (4-5) separable box blur (H then V) of the AO; (6) composite to
the backbuffer applying `lit * AO`, ACES tonemap, sRGB gamma, and the
horizon-gradient sky for background fragments (`depth >= 0.9999`). The main pass
samples the shadow map via `materials[i].maps[MATERIAL_MAP_BRDF]` for `DrawModel`
and via `SetShaderValueTexture` for rlgl immediate-mode primitives. Shaders live
in `assets/shaders/` and are copied to `build/assets/` by the existing
`POST_BUILD` step. The fullscreen post-process passes use an explicit fullscreen
quad with UVs in `0..1`; do not switch these passes back to `DrawRectangle()`
unless you also preserve correct texture coordinates and framebuffer sizing.

**Adding scene geometry.** The draw callback `DrawScene(Shader, void*)` in
`src/main.cpp` is invoked once per pass with the pass-appropriate shader. For
raylib models, assign `materials[i].shader = shader` before `DrawModel`. For
immediate-mode primitives, wrap in `BeginShaderMode(shader)`/`EndShaderMode`.
The callback must not call `BeginMode3D` - the pipeline sets up matrices.

**Ground rendering.** The ground is currently drawn as an immediate-mode textured
quad in `DrawGround()` rather than a model. Texture tiling is controlled by
`kGroundTextureTiles` in `src/main.cpp`, and the texture itself is
`assets/textures/grid-dark.png`. If tiling changes are needed, change the UVs or
that constant in the immediate-mode path rather than assuming mesh UV edits will
propagate automatically.

**Camera/input controls.** The app does not use raylib's built-in free camera
controls anymore. `UpdateCameraLook()` implements RMB-only mouselook (cursor
lock while held), and `UpdateCameraMovement()` implements `WASD` planar movement
plus `Space`/`Shift` vertical movement. If controls are changed, keep this custom
path authoritative rather than reintroducing `UpdateCamera(..., CAMERA_FREE)`.

**Physics direction.** The intended next step is integrating ReactPhysics3D as a
thin simulation layer, not replacing rendering. Preferred rollout:
1. Add/link the library in CMake.
2. Create a small `physics_world.{h,cpp}` wrapper around `PhysicsCommon` and `PhysicsWorld`.
3. Add a static ground body and a simple dynamic drone body (box collider first).
4. Step physics with a fixed timestep.
5. Drive render transforms from the physics body.
6. Add drone thrust/torque controls on top.

Do not start with mesh colliders or detailed aerodynamics.
