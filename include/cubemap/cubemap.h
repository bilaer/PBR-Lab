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
        void LoadKTXToCubemap(const std::string& path); // Load ktx file into cubemap texture
        void LoadEquiToCubemap(const std::string& path); // Load and convert hdr equirectangular image to cubemap.
        void SetCubemapTex(GLuint tex) { this->cubemap = tex; };
        void Bind(GLuint unit);
        void Unbind();
        GLuint GetTexture() const { return this->cubemap; };
        int GetMipLevels() const { return this->mipLevels; };
    protected:
        GLuint cubemap;
        GLenum internalFormat;
        GLenum format;
        GLenum type;
        int size;
        int mipLevels;
        int totalMipLevels;
        void InitializeCubemap();

};

