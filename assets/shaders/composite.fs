#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D litColor;
uniform sampler2D aoTex;
uniform sampler2D depthTex;
uniform float aoStrength;
uniform vec3 zenithColor;
uniform vec3 horizonColor;

vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec2 uv = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);
    float depth = texture(depthTex, uv).r;
    vec3 color;
    float ao;
    if (depth >= 0.9999) {
        float t = clamp(fragTexCoord.y, 0.0, 1.0);
        color = mix(horizonColor, zenithColor, pow(t, 1.5));
        ao = 1.0;
    } else {
        color = texture(litColor, uv).rgb;
        ao = texture(aoTex, uv).r;
        ao = mix(1.0, ao, aoStrength);
    }
    vec3 mapped = aces(color * ao);
    vec3 gamma = pow(mapped, vec3(1.0 / 2.2));
    finalColor = vec4(gamma, 1.0);
}
