#pragma once

#include "raylib.h"
#include "rlights.h"

using DrawSceneFn = void (*)(Shader shader, void* user);

struct Pipeline {
    int baseRenderW, baseRenderH;
    int renderW, renderH;

    Shader lighting;
    Shader shadowDepth;
    Shader ssao;
    Shader blur;
    Shader composite;

    unsigned int shadowFbo;
    unsigned int shadowDepthTex;
    int shadowSize;

    unsigned int gFbo;
    unsigned int gColorTex;
    unsigned int gDepthTex;

    unsigned int aoFbo;
    unsigned int aoColorTex;
    unsigned int aoBlurFbo;
    unsigned int aoBlurTex;

    Texture2D noiseTex;

    int viewPosLoc;
    int ambientLoc;
    int lightVPLoc;
    int sunIndexLoc;
    int shadowMapLoc;
    int shadowMapSizeLoc;

    int ssaoDepthLoc;
    int ssaoNoiseLoc;
    int ssaoSamplesLoc;
    int ssaoProjLoc;
    int ssaoInvProjLoc;
    int ssaoNoiseScaleLoc;
    int ssaoRadiusLoc;
    int ssaoBiasLoc;

    int blurTexLoc;
    int blurDirLoc;
    int blurTexelLoc;

    int compLitLoc;
    int compAoLoc;
    int compDepthLoc;
    int compAoStrengthLoc;
    int compZenithLoc;
    int compHorizonLoc;

    Light lights[LIGHTS_MAX];
    int lightCount;
    int sunIndex;
};

Pipeline PipelineCreate(int renderW, int renderH, float ssaaScale);
void PipelineDestroy(Pipeline& p);
void PipelineSetLights(Pipeline& p, const Light* lights, int count, int sunIndex);
void PipelineRender(Pipeline& p, Camera3D camera, DrawSceneFn drawScene, void* user);
