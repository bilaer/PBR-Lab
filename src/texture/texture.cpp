#include "texture/texture.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <cmath> 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gli/gli.hpp>
#include <gli/load_ktx.hpp>


Texture2D::Texture2D(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type):
    width(width),
    height(height),
    texture2d(0),
    internalFormat(internalFormat),
    format(format),
    type(type) {

    glGenTextures(1, &this->texture2d);
}

Texture2D::~Texture2D() {
    if(this->texture2d) {
        glDeleteTextures(1, &this->texture2d);
    }
}

void Texture2D::Bind(GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, this->texture2d);
}

void Texture2D::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Load hdr file to 2d texture
void Texture2D::LoadHDRToTexture(const std::string& path) {
    //stbi_set_flip_vertically_on_load(true);

    int w, h, comp; // comp: number of channels
    float* pixels = stbi_loadf(path.c_str(), &w, &h, &comp, 0);
    if (!pixels) {
        std::cerr << "[Texture2D] Failed to load HDR: " << path << "\n";
        return;
    }

    // Get format via the number of channels
    GLenum extFormat = GL_RED;
    switch (comp) {
        case 1: extFormat = GL_RED;  break;
        case 2: extFormat = GL_RG;   break;
        case 3: extFormat = GL_RGB;  break;
        case 4: extFormat = GL_RGBA; break;
        default: extFormat = format; break; 
    }
    if (extFormat != format) {
        // warning if format is not match
        std::cerr << "[Texture2D] Notice: file channels=" << comp
                  << ", using external format " << (int)extFormat
                  << " (internal stays " << (int)internalFormat << ")\n";
    }

    width  = static_cast<unsigned int>(w);
    height = static_cast<unsigned int>(h);

    glBindTexture(GL_TEXTURE_2D, texture2d);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Uplaod the image to texture
    glTexImage2D(GL_TEXTURE_2D, 0,
                 this->internalFormat, w, h, 0,
                 extFormat, GL_FLOAT, pixels);

    // Disable mipmaps by default, use CreateMipmaps and SetFilter if needed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Free data
    stbi_image_free(pixels);
}

// Load texture from ktx file to texture2d
void Texture2D::LoadKTXToTexture(const std::string& path) {
    gli::texture tex = gli::load_ktx(path.c_str());

    // Check if ktx loaded successfully 
    if (tex.empty()) {
        std::cerr << "[Texture2D] Failed to load KTX file: " << path << "\n";
        return;
    }

    // Get the width and height from the texture
    glm::uvec2 texSize = glm::uvec2(tex.extent(0).x, tex.extent(0).y);  // 只取宽度和高度
    this->width = texSize.x;
    this->height = texSize.y;

    glBindTexture(GL_TEXTURE_2D, this->texture2d);

    // Set internalFormat and format based on ktx texture format
    switch (tex.format()) {
        case gli::FORMAT_RGB32_SFLOAT_PACK32:
            internalFormat = GL_RGB32F;
            format = GL_RGB;
            break;
        case gli::FORMAT_RGBA32_SFLOAT_PACK32:
            internalFormat = GL_RGBA32F;
            format = GL_RGBA;
            break;
        case gli::FORMAT_RG32_SFLOAT_PACK32: // BRDF LUT
            internalFormat = GL_RG32F;
            format = GL_RG;
            break;
        case gli::FORMAT_RGB8_UNORM_PACK8:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        case gli::FORMAT_RGBA8_UNORM_PACK8:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            break;
        default:
            std::cerr << "[Texture2D] Unsupported KTX format, using default\n";
            break;
    }

    // Load texture
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texSize.x, texSize.y, 0, format, GL_FLOAT, tex.data(0, 0, 0));

    // Set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Check if texture is uploaded successfully
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "[Texture2D] Error occurred during texture upload\n";
    } else {
        std::cout << "[Texture2D] KTX texture successfully loaded: " << path << "\n";
    }
}

// Create empty 2d texture
void Texture2D::CreateStorage() {
    glBindTexture(GL_TEXTURE_2D, this->texture2d);
    glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, 
                this->width, this->height, 0, this->format, this->type, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create mipmaps to max level
void Texture2D::CreateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, this->texture2d);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Manually set the min and mag filter
void Texture2D::SetFilters(GLenum minFilter, GLenum magFilter) const {
    glBindTexture(GL_TEXTURE_2D, this->texture2d);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
}


