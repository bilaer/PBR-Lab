#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>


class Texture2D {
    public:
        Texture2D(unsigned int width, unsigned int height, GLenum internalFormat = GL_RGB32F, GLenum format = GL_RGB, GLenum type = GL_FLOAT);
        ~Texture2D();
        void Bind(GLuint unit);
        void Unbind();
        void CreateMipmaps();
        void SetFilters(GLenum minFilter, GLenum magFilter) const;
        void LoadHDRToTexture(const std::string& path);
        void LoadKTXToTexture(const std::string& path);
        GLuint GetTexture() const { return this->texture2d; };
    private:
        unsigned int width;
        unsigned int height;
        GLenum internalFormat;  // GL_RGB32F
        GLenum format;          // GL_RGB
        GLenum type;            // GL_FLOAT
        GLuint texture2d;
        void CreateStorage();
};