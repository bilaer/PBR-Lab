#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D brdfLUT;

void main() {
    vec2 brdf = texture(brdfLUT, TexCoords).rg;
    FragColor = vec4(brdf.r, brdf.g, 0.0, 1.0);  // 显示两个分量
}
