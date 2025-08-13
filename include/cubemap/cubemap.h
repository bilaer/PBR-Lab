#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <iostream>


// For HDR cubemap
// internal format: GL_RGB32F;
class Cubemap {
    public:
        Cubemap(unsigned int size, int mipLevels); // Create empty cubemap
        void LoadKTXToCubemap(const std::string& path);
        void SetCubemapTex(GLuint tex) { this->cubemap = tex; };
        void Bind(GLuint unit);
        void Unbind();
        GLuint GetTexture() const { return this->cubemap; };
    protected:
        GLuint cubemap;
        GLenum internalFormat;
        GLenum format;
        int size;
        int mipLevels;
        int totalMipLevels;
        void InitializeCubemap();

};

