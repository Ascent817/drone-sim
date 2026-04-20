#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D depthTex;
uniform sampler2D noiseTex;
uniform vec3 samples[16];
uniform mat4 projection;
uniform mat4 invProjection;
uniform vec2 noiseScale;
uniform float radius;
uniform float bias;

vec3 viewPosFromDepth(vec2 uv, float depth) {
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = invProjection * clip;
    return view.xyz / view.w;
}

void main() {
    float depth = texture(depthTex, fragTexCoord).r;
    if (depth >= 1.0) {
        finalColor = vec4(1.0);
        return;
    }
    vec3 viewPos = viewPosFromDepth(fragTexCoord, depth);

    vec3 ddxPos = dFdx(viewPos);
    vec3 ddyPos = dFdy(viewPos);
    vec3 N = normalize(cross(ddxPos, ddyPos));

    vec3 randomVec = normalize(texture(noiseTex, fragTexCoord * noiseScale).xyz * 2.0 - 1.0);
    vec3 T = normalize(randomVec - N * dot(randomVec, N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    float occlusion = 0.0;
    for (int i = 0; i < 16; i++) {
        vec3 samplePos = TBN * samples[i];
        samplePos = viewPos + samplePos * radius;

        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        vec2 sampleUV = offset.xy * 0.5 + 0.5;

        float sampleDepth = texture(depthTex, sampleUV).r;
        if (sampleDepth >= 1.0) continue;
        vec3 occluderView = viewPosFromDepth(sampleUV, sampleDepth);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - occluderView.z));
        occlusion += (occluderView.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    float ao = 1.0 - (occlusion / 16.0);
    ao = pow(clamp(ao, 0.0, 1.0), 1.8);
    finalColor = vec4(ao, ao, ao, 1.0);
}
