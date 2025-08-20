#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include "geometry.h"


class Texture2D {
    public:
        // Constructor provide default values for hdr format
        Texture2D(unsigned int width, unsigned int height, GLenum internalFormat = GL_RGB32F, GLenum format = GL_RGB, GLenum type = GL_FLOAT);
        Texture2D(); // Used for create empty texture, paramters will be set when using load function
        ~Texture2D();
        void Bind(GLuint unit);
        void Unbind();
        void CreateMipmaps();
        void SetFilters(GLenum minFilter, GLenum magFilter) const;

        // Load functions
        void LoadHDRToTexture(const std::string& path, bool flipY = false);
        void LoadKTXToTexture(const std::string& path);
        void LoadLDRToTexture(const std::string& path,  bool isSRGB, bool flipY = false);

        // Load texture directly pixels
        void CreateFromPixels(const unsigned char* pixels, int w, int h, int comp, bool isSRGB);

        // Texture getter and setter
        GLuint GetTexture() const { return this->texture2d; };
        void SetTexture(GLuint id) { this->texture2d = id; };
        void ShowTexture2D(GLFWwindow* sharedContext); // Display 2d texture for debuggin purpose
    private:
        unsigned int width;
        unsigned int height;
        GLenum internalFormat;  // GL_RGB32F
        GLenum format;          // GL_RGB
        GLenum type;            // GL_FLOAT
        GLuint texture2d;
        void CreateStorage();
};