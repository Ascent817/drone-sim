# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Bootstrap of a 3D drone simulation built on raylib + C++17. Currently a single-file
scene (`src/main.cpp`) with a free-fly camera, grid floor, and a drone model loader
that falls back to a placeholder cube + rotor blocks when no asset is present.
Subsequent work (physics, controls, telemetry) builds on top of this skeleton.

## Toolchain (Windows)

The Machine PATH has been ordered so the right tools win in any fresh shell:

- `C:\Program Files\CMake\bin` (CMake 3.25.2) — position 0 on Machine PATH.
- `C:\ProgramData\mingw64\mingw64\bin` (MinGW-w64 GCC 15.2.0, `choco install mingw`) — position 1.
- `C:\MinGW\bin` (legacy GCC 6.3.0 from 2016) — still present but deprioritized. Do **not**
  link the pre-built raylib against this old toolchain; it produces libgcc/libstdc++
  ABI errors. If a build picks it up, fix PATH order rather than working around it.

Already-running shells (including this bash session) inherited the old env and won't
see the change until restart. New shells are fine.

## Build & run

First-time configure (and any time `CMakeLists.txt` changes):
```
cmake -S . -B build -G "MinGW Makefiles"
```

Incremental build + run:
```
cmake --build build
./build/drone.exe
```

There are no tests yet. The smoke check is: launch `drone.exe`, confirm a 1280×720
window opens with a grid + placeholder drone, the free-fly camera responds to WASD +
mouse, and the process stays alive.

## Architecture

**Vendored raylib, not fetched.** `vendor/raylib/` contains the official pre-built
MinGW-w64 5.5 release (`include/`, `lib/libraylib.a`). CMake links the **static**
`libraylib.a` plus the Windows system libs raylib needs (`opengl32 gdi32 winmm`),
so `drone.exe` runs without shipping `raylib.dll`. There is no `find_package` /
`FetchContent` — pointing CMake at `vendor/raylib` keeps the build hermetic and
offline. If you upgrade raylib, replace the contents of `vendor/raylib/` in place.

**Asset pipeline.** `assets/` at the project root is the source of truth. A
`POST_BUILD` step in `CMakeLists.txt` copies the whole directory next to the built
executable (`build/assets/`), so `LoadModel("assets/drone.glb")` resolves whether the
exe is launched from the project root or from `build/`. Don't edit `build/assets/`
— it's regenerated every build. Drop new model files into `assets/` and rebuild.

**Model loading is fallback-driven.** `TryLoadDroneModel` in `src/main.cpp` checks
for `assets/drone.glb` first, then `assets/drone.obj`, and reports success via
`model.meshCount > 0` (raylib's documented signal — a failed `LoadModel` returns a
zero-mesh `Model`, not an error code). When no model loads, `DrawPlaceholderDrone`
renders a cube body + four rotor cubes so the scene is always usable. Any model
work should preserve this fallback so a missing asset never crashes the app.
