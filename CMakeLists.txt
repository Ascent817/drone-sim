cmake_minimum_required(VERSION 3.15)
project(drone CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/vendor/raylib")
add_subdirectory(vendor/reactphysics3d)

add_executable(drone
    src/main.cpp
    src/render_pipeline.cpp
    src/physics_world.cpp
    src/entities/camera.cpp
    src/entities/drone.cpp
    src/entities/groundplane.cpp
)

target_include_directories(drone PRIVATE "${RAYLIB_DIR}/include")
target_link_directories(drone PRIVATE "${RAYLIB_DIR}/lib")
target_link_libraries(drone PRIVATE raylib reactphysics3d opengl32 gdi32 winmm)

add_custom_command(TARGET drone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:drone>/assets"
)
