#version 330 core
out vec4 FragColor;
in vec3 WorldPos; 
uniform sampler2D envEqui;

const float PI      = 3.14159265359;
const float TWO_PI  = 6.28318530718;
const float HALF_PI = 1.57079632679;

vec2 DirtoEquiUV(vec3 direction) {
    vec3 nDir = normalize(direction);

    float theta = atan(nDir.z, nDir.x);
    float phi   = asin(clamp(nDir.y, -1.0, 1.0));

    float u = theta / TWO_PI + 0.5;
    float v = 0.5 - phi / PI;

    return vec2(u, v);
}

void main() {
    vec3 normal = normalize(WorldPos);
    vec2 uv = DirtoEquiUV(normal);
    vec3 color = texture(envEqui, uv).rgb;
    FragColor = vec4(color, 1.0);
}
