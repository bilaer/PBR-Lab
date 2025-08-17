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
    if (this->texture2d == 0) {
        std::cerr << "❌ Failed to generate texture!" << std::endl;
    }
}

Texture2D::Texture2D() {
    glGenTextures(1, &this->texture2d);
    if (this->texture2d == 0) {
        std::cerr << "❌ Failed to generate texture!" << std::endl;
    }
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
void Texture2D::LoadHDRToTexture(const std::string& path, bool flipY) {
    stbi_set_flip_vertically_on_load(flipY);

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
    // Set formats to match image format
    this->format = extFormat;

    width  = static_cast<unsigned int>(w);
    height = static_cast<unsigned int>(h);

    glBindTexture(GL_TEXTURE_2D, this->texture2d);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Uplaod the image to texture
    glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, w, h, 0, this->format, GL_FLOAT, pixels);

    // Disable mipmaps by default, use CreateMipmaps and SetFilter if needed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Check if texture is uploaded successfully
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "[Texture2D] Error occurred during texture upload\n";
    } else {
        std::cout << "[Texture2D] successfully loaded HDR texture : " << path << "\n";
    }

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
            this->internalFormat = GL_RGB32F;
            this->format = GL_RGB;
            break;
        case gli::FORMAT_RGBA32_SFLOAT_PACK32:
            this->internalFormat = GL_RGBA32F;
            this->format = GL_RGBA;
            break;
        case gli::FORMAT_RG32_SFLOAT_PACK32: // BRDF LUT
            this->internalFormat = GL_RG32F;
            this->format = GL_RG;
            break;
        case gli::FORMAT_RGB8_UNORM_PACK8:
            this->internalFormat = GL_RGB8;
            this->format = GL_RGB;
            break;
        case gli::FORMAT_RGBA8_UNORM_PACK8:
            this->internalFormat = GL_RGBA8;
            this->format = GL_RGBA;
            break;
        default:
            std::cerr << "[Texture2D] Unsupported KTX format, using default\n";
            break;
    }


    // Load texture
    glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, texSize.x, texSize.y, 0, this->format, GL_FLOAT, tex.data(0, 0, 0));

    // Set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Check if texture is uploaded successfully
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "[Texture2D] Error occurred during texture upload\n";
    } else {
        std::cout << "[Texture2D] successfully loaded KTX texture : " << path << "\n";
    }
}

// Load LDR (jpg, png...) to texture 2d
void Texture2D::LoadLDRToTexture(const std::string& path, bool flipY) {
    // Whether flip y axis, used when texture is upside down
    stbi_set_flip_vertically_on_load(flipY);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "❌ Failed to load texture: " << path << std::endl;
    }

    GLenum format;
    if (nrChannels == 1) {
        format = GL_RED;
    } else if (nrChannels == 3) {
        format = GL_RGB;
    } else if (nrChannels == 4) {
        format = GL_RGBA;
    } else {
        std::cerr << "⚠️ Unsupported channel count: " << nrChannels << std::endl;
        stbi_image_free(data);
    }

    glBindTexture(GL_TEXTURE_2D, this->texture2d); // Bind texture

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set formats to match image format
    this->internalFormat = format;
    this->format = format;

    // Set paramters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
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

// Display texture 2D for debugging purpose
void Texture2D::ShowTexture2D(GLFWwindow* sharedContext) {
    // Window creation hints (affects only the next created window)
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (this->texture2d == 0) {
        std::cerr << "❌ texture2d is 0. Abort debug viewer.\n";
        return;
    }

    // Create a shared-context debug window
    GLFWwindow* debugWindow = glfwCreateWindow(512, 512, "BRDF LUT Viewer", nullptr, sharedContext);
    if (!debugWindow) {
        std::cerr << "❌ Failed to create BRDF LUT Debug window!\n";
        return;
    }

    glfwMakeContextCurrent(debugWindow);
    glfwSwapInterval(1);  // Enable V-Sync

    // Create shader and quad in the debug window's context
    auto debugShader = std::make_shared<Shader>("shader/show_texture2d.vert",
                                                "shader/show_texture2d.frag");
    auto screenQuad  = std::make_shared<ScreenQuad>(); // Ensure VAO/VBO is created in the same context

    // Initial viewport setup
    int fbw, fbh;
    glfwGetFramebufferSize(debugWindow, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    // Main Loop
    while (!glfwWindowShouldClose(debugWindow)) {
        if (glfwGetKey(debugWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(debugWindow, true);
        }

        // Update viewport if the window size changes
        int newW, newH;
        glfwGetFramebufferSize(debugWindow, &newW, &newH);
        if (newW != fbw || newH != fbh) {
            fbw = newW; fbh = newH;
            glViewport(0, 0, fbw, fbh);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        debugShader->Use();
        debugShader->SetUniform("tex", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->texture2d);

        screenQuad->Draw();

        glfwSwapBuffers(debugWindow);
        glfwPollEvents();
    }

    // --- Make sure OpenGL resources are deleted while the context is still current ---
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Release GL resources created in this function
    screenQuad.reset();   // If destructor calls glDelete*, it must happen in a valid context
    debugShader.reset();  // Same here, ensures glDeleteProgram is called safely

    // Optionally clear the current context before destroying the window
    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(debugWindow);
}



