#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 direction;
uniform vec2 texelSize;

void main() {
    vec2 off = direction * texelSize;
    float c = 0.0;
    c += texture(texture0, fragTexCoord - 2.0 * off).r;
    c += texture(texture0, fragTexCoord - 1.0 * off).r;
    c += texture(texture0, fragTexCoord).r;
    c += texture(texture0, fragTexCoord + 1.0 * off).r;
    c += texture(texture0, fragTexCoord + 2.0 * off).r;
    c /= 5.0;
    finalColor = vec4(c, c, c, 1.0);
}
