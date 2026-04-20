#version 330

in vec3 fragPositionWorld;
in vec3 fragPositionView;
in vec3 fragNormalWorld;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec4 fragPositionLight;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec4 ambient;

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};
#define LIGHTS_MAX 4
uniform Light lights[LIGHTS_MAX];
uniform int sunIndex;

uniform sampler2D shadowMap;
uniform vec2 shadowMapSize;

out vec4 finalColor;

float sampleShadow(vec4 lightSpacePos) {
    vec3 proj = lightSpacePos.xyz / lightSpacePos.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 1.0;
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) return 1.0;

    float bias = 0.0015;
    float currentDepth = proj.z - bias;

    float shadow = 0.0;
    vec2 texel = 1.0 / shadowMapSize;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, proj.xy + vec2(x, y) * texel).r;
            shadow += currentDepth > pcfDepth ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec4 texel = texture(texture0, fragTexCoord) * colDiffuse * fragColor;

    vec3 N = fragNormalWorld;
    float nlen = length(N);
    if (nlen < 0.001) {
        finalColor = texel;
        return;
    }
    N /= nlen;
    vec3 V = normalize(viewPos - fragPositionWorld);

    vec3 lighting = ambient.rgb * texel.rgb;

    for (int i = 0; i < LIGHTS_MAX; i++) {
        if (lights[i].enabled == 0) continue;
        vec3 L;
        if (lights[i].type == 0) {
            L = normalize(lights[i].position - lights[i].target);
        } else {
            L = normalize(lights[i].position - fragPositionWorld);
        }
        float ndotl = max(dot(N, L), 0.0);
        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), 64.0) * (ndotl > 0.0 ? 1.0 : 0.0);

        float visibility = 1.0;
        if (i == sunIndex) {
            visibility = sampleShadow(fragPositionLight);
        }

        vec3 contrib = lights[i].color.rgb * (ndotl * texel.rgb + spec * vec3(0.6));
        lighting += contrib * visibility;
    }

    finalColor = vec4(lighting, texel.a);
}
