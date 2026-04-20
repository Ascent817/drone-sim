#include "render_pipeline.h"

#include "raymath.h"
#include "rlgl.h"

#include <cmath>
#include <random>

namespace {

float FRand(std::mt19937& rng) {
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
}

void BindFramebufferTarget(unsigned int fboId, int framebufferW, int framebufferH) {
    rlDrawRenderBatchActive();
    if (fboId != 0) rlEnableFramebuffer(fboId);
    else rlDisableFramebuffer();
    rlViewport(0, 0, framebufferW, framebufferH);
    rlSetFramebufferWidth(framebufferW);
    rlSetFramebufferHeight(framebufferH);
}

void SetupOrtho2D(int drawW, int drawH) {
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0.0, (double)drawW, (double)drawH, 0.0, 0.0, 1.0);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void DrawFullscreenQuad(int drawW, int drawH) {
    rlSetTexture(rlGetTextureIdDefault());
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex2f(0.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f);
    rlVertex2f(0.0f, (float)drawH);
    rlTexCoord2f(1.0f, 1.0f);
    rlVertex2f((float)drawW, (float)drawH);
    rlTexCoord2f(1.0f, 0.0f);
    rlVertex2f((float)drawW, 0.0f);
    rlEnd();
    rlSetTexture(0);
}

}  // namespace

Pipeline PipelineCreate(int renderW, int renderH, float ssaaScale) {
    Pipeline p{};
    p.baseRenderW = renderW;
    p.baseRenderH = renderH;
    p.renderW = (int)std::lround((double)renderW * ssaaScale);
    p.renderH = (int)std::lround((double)renderH * ssaaScale);

    p.lighting    = LoadShader("assets/shaders/lighting.vs",    "assets/shaders/lighting.fs");
    p.shadowDepth = LoadShader("assets/shaders/shadow_depth.vs","assets/shaders/shadow_depth.fs");
    p.ssao        = LoadShader("assets/shaders/passthrough.vs", "assets/shaders/ssao.fs");
    p.blur        = LoadShader("assets/shaders/passthrough.vs", "assets/shaders/blur.fs");
    p.composite   = LoadShader("assets/shaders/passthrough.vs", "assets/shaders/composite.fs");

    p.viewPosLoc       = GetShaderLocation(p.lighting, "viewPos");
    p.ambientLoc       = GetShaderLocation(p.lighting, "ambient");
    p.lightVPLoc       = GetShaderLocation(p.lighting, "lightVP");
    p.sunIndexLoc      = GetShaderLocation(p.lighting, "sunIndex");
    p.shadowMapLoc     = GetShaderLocation(p.lighting, "shadowMap");
    p.shadowMapSizeLoc = GetShaderLocation(p.lighting, "shadowMapSize");
    p.lighting.locs[SHADER_LOC_VECTOR_VIEW]   = p.viewPosLoc;
    p.lighting.locs[SHADER_LOC_MATRIX_MODEL]  = GetShaderLocation(p.lighting, "matModel");
    p.lighting.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(p.lighting, "matNormal");
    p.lighting.locs[SHADER_LOC_MATRIX_VIEW]   = GetShaderLocation(p.lighting, "matView");
    p.lighting.locs[SHADER_LOC_MAP_BRDF]      = p.shadowMapLoc;

    p.ssaoDepthLoc      = GetShaderLocation(p.ssao, "depthTex");
    p.ssaoNoiseLoc      = GetShaderLocation(p.ssao, "noiseTex");
    p.ssaoSamplesLoc    = GetShaderLocation(p.ssao, "samples");
    p.ssaoProjLoc       = GetShaderLocation(p.ssao, "projection");
    p.ssaoInvProjLoc    = GetShaderLocation(p.ssao, "invProjection");
    p.ssaoNoiseScaleLoc = GetShaderLocation(p.ssao, "noiseScale");
    p.ssaoRadiusLoc     = GetShaderLocation(p.ssao, "radius");
    p.ssaoBiasLoc       = GetShaderLocation(p.ssao, "bias");

    p.blurTexLoc   = GetShaderLocation(p.blur, "texture0");
    p.blurDirLoc   = GetShaderLocation(p.blur, "direction");
    p.blurTexelLoc = GetShaderLocation(p.blur, "texelSize");

    p.compLitLoc         = GetShaderLocation(p.composite, "litColor");
    p.compAoLoc          = GetShaderLocation(p.composite, "aoTex");
    p.compDepthLoc       = GetShaderLocation(p.composite, "depthTex");
    p.compAoStrengthLoc  = GetShaderLocation(p.composite, "aoStrength");
    p.compZenithLoc      = GetShaderLocation(p.composite, "zenithColor");
    p.compHorizonLoc     = GetShaderLocation(p.composite, "horizonColor");

    p.shadowSize = 2048;
    p.shadowFbo = rlLoadFramebuffer();
    rlEnableFramebuffer(p.shadowFbo);
    p.shadowDepthTex = rlLoadTextureDepth(p.shadowSize, p.shadowSize, false);
    rlTextureParameters(p.shadowDepthTex, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
    rlTextureParameters(p.shadowDepthTex, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
    rlTextureParameters(p.shadowDepthTex, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(p.shadowDepthTex, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
    rlFramebufferAttach(p.shadowFbo, p.shadowDepthTex,
                        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
    if (!rlFramebufferComplete(p.shadowFbo)) {
        TraceLog(LOG_WARNING, "Shadow FBO incomplete");
    }
    rlDisableFramebuffer();

    p.gFbo = rlLoadFramebuffer();
    rlEnableFramebuffer(p.gFbo);
    p.gColorTex = rlLoadTexture(nullptr, p.renderW, p.renderH,
                                RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    rlTextureParameters(p.gColorTex, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
    rlTextureParameters(p.gColorTex, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
    rlTextureParameters(p.gColorTex, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(p.gColorTex, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
    p.gDepthTex = rlLoadTextureDepth(p.renderW, p.renderH, false);
    rlTextureParameters(p.gDepthTex, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
    rlTextureParameters(p.gDepthTex, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
    rlTextureParameters(p.gDepthTex, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(p.gDepthTex, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
    rlFramebufferAttach(p.gFbo, p.gColorTex,
                        RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(p.gFbo, p.gDepthTex,
                        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
    if (!rlFramebufferComplete(p.gFbo)) {
        TraceLog(LOG_WARNING, "G-buffer FBO incomplete");
    }
    rlDisableFramebuffer();

    auto makeColorOnlyFbo = [&](unsigned int& fboOut, unsigned int& texOut) {
        fboOut = rlLoadFramebuffer();
        rlEnableFramebuffer(fboOut);
        texOut = rlLoadTexture(nullptr, p.renderW, p.renderH,
                               RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        rlTextureParameters(texOut, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlTextureParameters(texOut, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlTextureParameters(texOut, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
        rlTextureParameters(texOut, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
        rlFramebufferAttach(fboOut, texOut,
                            RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        if (!rlFramebufferComplete(fboOut)) {
            TraceLog(LOG_WARNING, "AO FBO incomplete");
        }
        rlDisableFramebuffer();
    };
    makeColorOnlyFbo(p.aoFbo, p.aoColorTex);
    makeColorOnlyFbo(p.aoBlurFbo, p.aoBlurTex);

    std::mt19937 rng(42);
    float kernel[16 * 3];
    for (int i = 0; i < 16; i++) {
        Vector3 s{FRand(rng) * 2.0f - 1.0f, FRand(rng) * 2.0f - 1.0f, FRand(rng)};
        s = Vector3Normalize(s);
        s = Vector3Scale(s, FRand(rng));
        float t = (float)i / 16.0f;
        float scale = 0.1f + 0.9f * (t * t);
        s = Vector3Scale(s, scale);
        kernel[i * 3 + 0] = s.x;
        kernel[i * 3 + 1] = s.y;
        kernel[i * 3 + 2] = s.z;
    }
    SetShaderValueV(p.ssao, p.ssaoSamplesLoc, kernel, SHADER_UNIFORM_VEC3, 16);

    float radius = 0.6f;
    float bias = 0.025f;
    SetShaderValue(p.ssao, p.ssaoRadiusLoc, &radius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(p.ssao, p.ssaoBiasLoc, &bias, SHADER_UNIFORM_FLOAT);
    float noiseScale[2] = {(float)p.renderW / 4.0f, (float)p.renderH / 4.0f};
    SetShaderValue(p.ssao, p.ssaoNoiseScaleLoc, noiseScale, SHADER_UNIFORM_VEC2);

    Color noisePx[16];
    for (int i = 0; i < 16; i++) {
        float nx = FRand(rng) * 2.0f - 1.0f;
        float ny = FRand(rng) * 2.0f - 1.0f;
        noisePx[i] = Color{
            (unsigned char)((nx * 0.5f + 0.5f) * 255.0f),
            (unsigned char)((ny * 0.5f + 0.5f) * 255.0f),
            (unsigned char)(0.5f * 255.0f),
            255,
        };
    }
    Image noiseImg{noisePx, 4, 4, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    p.noiseTex = LoadTextureFromImage(noiseImg);
    SetTextureFilter(p.noiseTex, TEXTURE_FILTER_POINT);
    SetTextureWrap(p.noiseTex, TEXTURE_WRAP_REPEAT);

    float aoStrength = 1.0f;
    SetShaderValue(p.composite, p.compAoStrengthLoc, &aoStrength, SHADER_UNIFORM_FLOAT);
    float zenith[3]  = {0.35f, 0.55f, 0.85f};
    float horizon[3] = {0.82f, 0.86f, 0.93f};
    SetShaderValue(p.composite, p.compZenithLoc, zenith, SHADER_UNIFORM_VEC3);
    SetShaderValue(p.composite, p.compHorizonLoc, horizon, SHADER_UNIFORM_VEC3);

    float ambient[4] = {0.20f, 0.22f, 0.28f, 1.0f};
    SetShaderValue(p.lighting, p.ambientLoc, ambient, SHADER_UNIFORM_VEC4);

    return p;
}

void PipelineDestroy(Pipeline& p) {
    UnloadShader(p.lighting);
    UnloadShader(p.shadowDepth);
    UnloadShader(p.ssao);
    UnloadShader(p.blur);
    UnloadShader(p.composite);
    UnloadTexture(p.noiseTex);
    rlUnloadTexture(p.shadowDepthTex);
    rlUnloadTexture(p.gColorTex);
    rlUnloadTexture(p.gDepthTex);
    rlUnloadTexture(p.aoColorTex);
    rlUnloadTexture(p.aoBlurTex);
    rlUnloadFramebuffer(p.shadowFbo);
    rlUnloadFramebuffer(p.gFbo);
    rlUnloadFramebuffer(p.aoFbo);
    rlUnloadFramebuffer(p.aoBlurFbo);
}

void PipelineSetLights(Pipeline& p, const Light* lights, int count, int sunIndex) {
    p.lightCount = count < LIGHTS_MAX ? count : LIGHTS_MAX;
    p.sunIndex = sunIndex;
    for (int i = 0; i < p.lightCount; i++) p.lights[i] = lights[i];
    SetShaderValue(p.lighting, p.sunIndexLoc, &sunIndex, SHADER_UNIFORM_INT);
}

void PipelineRender(Pipeline& p, Camera3D camera, DrawSceneFn drawScene, void* user) {
    const Light& sun = p.lights[p.sunIndex];
    Matrix lightView = MatrixLookAt(sun.position, sun.target, Vector3{0.0f, 1.0f, 0.0f});
    float sz = 15.0f;
    Matrix lightProj = MatrixOrtho(-sz, sz, -sz, sz, 0.1, 60.0);
    Matrix lightVP   = MatrixMultiply(lightProj, lightView);

    // Pass 1: shadow depth
    BindFramebufferTarget(p.shadowFbo, p.shadowSize, p.shadowSize);
    rlClearScreenBuffers();
    rlEnableDepthTest();
    rlDisableColorBlend();

    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();
    rlSetMatrixProjection(lightProj);
    rlMatrixMode(RL_MODELVIEW);
    rlPushMatrix();
    rlLoadIdentity();
    rlSetMatrixModelview(lightView);

    drawScene(p.shadowDepth, user);
    rlDrawRenderBatchActive();

    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();
    rlMatrixMode(RL_MODELVIEW);
    rlPopMatrix();

    BindFramebufferTarget(0, p.baseRenderW, p.baseRenderH);
    rlEnableColorBlend();

    // Pass 2: main lit geometry into G-buffer
    float viewPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(p.lighting, p.viewPosLoc, viewPos, SHADER_UNIFORM_VEC3);
    SetShaderValueMatrix(p.lighting, p.lightVPLoc, lightVP);
    float shadowMapSize[2] = {(float)p.shadowSize, (float)p.shadowSize};
    SetShaderValue(p.lighting, p.shadowMapSizeLoc, shadowMapSize, SHADER_UNIFORM_VEC2);
    for (int i = 0; i < p.lightCount; i++) UpdateLightValues(p.lighting, p.lights[i]);

    Texture2D shadowTex{};
    shadowTex.id = p.shadowDepthTex;
    shadowTex.width = p.shadowSize;
    shadowTex.height = p.shadowSize;
    shadowTex.mipmaps = 1;
    shadowTex.format = PIXELFORMAT_UNCOMPRESSED_R32;

    BindFramebufferTarget(p.gFbo, p.renderW, p.renderH);
    ClearBackground(BLANK);
    BeginMode3D(camera);

    Matrix capturedProjection = rlGetMatrixProjection();

    SetShaderValueTexture(p.lighting, p.shadowMapLoc, shadowTex);
    drawScene(p.lighting, user);

    EndMode3D();
    rlDrawRenderBatchActive();

    // Pass 3: SSAO
    Matrix invProj = MatrixInvert(capturedProjection);
    SetShaderValueMatrix(p.ssao, p.ssaoProjLoc, capturedProjection);
    SetShaderValueMatrix(p.ssao, p.ssaoInvProjLoc, invProj);

    Texture2D gDepth{};
    gDepth.id = p.gDepthTex;
    gDepth.width = p.renderW;
    gDepth.height = p.renderH;
    gDepth.mipmaps = 1;
    gDepth.format = PIXELFORMAT_UNCOMPRESSED_R32;

    BindFramebufferTarget(p.aoFbo, p.renderW, p.renderH);
    SetupOrtho2D(p.renderW, p.renderH);
    ClearBackground(WHITE);
    BeginShaderMode(p.ssao);
    SetShaderValueTexture(p.ssao, p.ssaoDepthLoc, gDepth);
    SetShaderValueTexture(p.ssao, p.ssaoNoiseLoc, p.noiseTex);
    DrawFullscreenQuad(p.renderW, p.renderH);
    EndShaderMode();
    rlDrawRenderBatchActive();

    // Pass 4: blur AO (H then V)
    Texture2D aoTex{};
    aoTex.id = p.aoColorTex;
    aoTex.width = p.renderW;
    aoTex.height = p.renderH;
    aoTex.mipmaps = 1;
    aoTex.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    Texture2D aoBlurTex{};
    aoBlurTex.id = p.aoBlurTex;
    aoBlurTex.width = p.renderW;
    aoBlurTex.height = p.renderH;
    aoBlurTex.mipmaps = 1;
    aoBlurTex.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    float texel[2] = {1.0f / (float)p.renderW, 1.0f / (float)p.renderH};
    SetShaderValue(p.blur, p.blurTexelLoc, texel, SHADER_UNIFORM_VEC2);

    BindFramebufferTarget(p.aoBlurFbo, p.renderW, p.renderH);
    SetupOrtho2D(p.renderW, p.renderH);
    ClearBackground(WHITE);
    BeginShaderMode(p.blur);
    SetShaderValueTexture(p.blur, p.blurTexLoc, aoTex);
    float hDir[2] = {1.0f, 0.0f};
    SetShaderValue(p.blur, p.blurDirLoc, hDir, SHADER_UNIFORM_VEC2);
    DrawFullscreenQuad(p.renderW, p.renderH);
    EndShaderMode();
    rlDrawRenderBatchActive();

    BindFramebufferTarget(p.aoFbo, p.renderW, p.renderH);
    SetupOrtho2D(p.renderW, p.renderH);
    ClearBackground(WHITE);
    BeginShaderMode(p.blur);
    SetShaderValueTexture(p.blur, p.blurTexLoc, aoBlurTex);
    float vDir[2] = {0.0f, 1.0f};
    SetShaderValue(p.blur, p.blurDirLoc, vDir, SHADER_UNIFORM_VEC2);
    DrawFullscreenQuad(p.renderW, p.renderH);
    EndShaderMode();
    rlDrawRenderBatchActive();

    // Pass 5: composite to backbuffer (already in BeginDrawing scope)
    Texture2D gColor{};
    gColor.id = p.gColorTex;
    gColor.width = p.renderW;
    gColor.height = p.renderH;
    gColor.mipmaps = 1;
    gColor.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    BindFramebufferTarget(0, p.baseRenderW, p.baseRenderH);
    SetupOrtho2D(p.baseRenderW, p.baseRenderH);
    BeginShaderMode(p.composite);
    SetShaderValueTexture(p.composite, p.compLitLoc, gColor);
    SetShaderValueTexture(p.composite, p.compAoLoc, aoTex);
    SetShaderValueTexture(p.composite, p.compDepthLoc, gDepth);
    DrawFullscreenQuad(p.baseRenderW, p.baseRenderH);
    EndShaderMode();
}
