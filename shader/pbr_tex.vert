#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in vec3 aTangent;    // 来自 VAO
layout(location=4) in vec3 aBitangent;  // 来自 VAO

out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 TangentWS;
out vec3 BitangentWS;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transform to world space
    vec3 N = normalize(mat3(model) * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);
    // Gram-Schmidt orthogonalization to keep T orthogonal to N
    T = normalize(T - N * dot(N, T));
    vec3 B = normalize(mat3(model) * aBitangent);

    Normal      = N;
    TangentWS   = T;
    BitangentWS = B;
    TexCoords   = aUV;
    WorldPos    = vec3(model * vec4(aPos, 1.0));

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
