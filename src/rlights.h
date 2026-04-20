#pragma once

#include "raylib.h"
#include <cstdio>

#define LIGHTS_MAX 4

enum LightType { LIGHT_DIRECTIONAL = 0, LIGHT_POINT = 1 };

struct Light {
    int type;
    bool enabled;
    Vector3 position;
    Vector3 target;
    Color color;
    int enabledLoc;
    int typeLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
};

inline Light CreateLight(int index, int type, Vector3 position, Vector3 target, Color color, Shader shader) {
    Light light{};
    light.enabled = true;
    light.type = type;
    light.position = position;
    light.target = target;
    light.color = color;

    char name[64];
    std::snprintf(name, sizeof(name), "lights[%d].enabled", index);
    light.enabledLoc = GetShaderLocation(shader, name);
    std::snprintf(name, sizeof(name), "lights[%d].type", index);
    light.typeLoc = GetShaderLocation(shader, name);
    std::snprintf(name, sizeof(name), "lights[%d].position", index);
    light.positionLoc = GetShaderLocation(shader, name);
    std::snprintf(name, sizeof(name), "lights[%d].target", index);
    light.targetLoc = GetShaderLocation(shader, name);
    std::snprintf(name, sizeof(name), "lights[%d].color", index);
    light.colorLoc = GetShaderLocation(shader, name);
    return light;
}

inline void UpdateLightValues(Shader shader, const Light& light) {
    int enabled = light.enabled ? 1 : 0;
    SetShaderValue(shader, light.enabledLoc, &enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);
    float pos[3] = {light.position.x, light.position.y, light.position.z};
    SetShaderValue(shader, light.positionLoc, pos, SHADER_UNIFORM_VEC3);
    float target[3] = {light.target.x, light.target.y, light.target.z};
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
    float color[4] = {
        light.color.r / 255.0f, light.color.g / 255.0f,
        light.color.b / 255.0f, light.color.a / 255.0f,
    };
    SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
}
