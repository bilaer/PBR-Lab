#version 330 core

layout (location = 0) in vec3 aPos;  // object vertex
layout (location = 1) in vec3 aNormal;  // normal
layout (location = 2) in vec2 aTexCoords;  // object's uv

out vec3 WorldPos;  
out vec3 Normal;    
out vec2 TexCoords;  
out mat3 TBN;

uniform mat4 model;  // model matrix
uniform mat4 view;   // view matrix
uniform mat4 projection;  // projection matrix

void main() {

    WorldPos = vec3(model * vec4(aPos, 1.0));
    
    // Calculate normal
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
