#include "cubemap/cubemap.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <string>
#include <vector>
#include <cmath>
#include <filesystem>
#include <unordered_map>
#include <array> 
#include <algorithm> 
#include <gli/gli.hpp>
#include <gli/load_ktx.hpp>
namespace fs = std::filesystem;

// Initialize an empty cubemap
Cubemap::Cubemap(unsigned int size, int mipLevels): size(size), mipLevels(mipLevels) {
    this->totalMipLevels = static_cast<int>(std::floor(std::log2(this->size))) + 1;
    this->InitializeCubemap();
}

// Load KTX texture file into cubemap
void Cubemap::LoadKTXToCubemap(const std::string& path) {
    // 1) 读 KTX 到通用 texture
    gli::texture tex = gli::load_ktx(path);
    if (tex.empty()) {
        std::cerr << "Failed to load KTX file: " << path << std::endl;
        return;
    }

    // 2) 必须是 cubemap：一般 faces()==6；有些 KTX 还会把 target 标注成 CUBE
    if (tex.faces() != 6) {
        std::cerr << "Loaded texture is not a cubemap! faces=" << tex.faces() << std::endl;
        return;
    }

    // 3) 基于同一块 storage 创建一个 cube 视图（不拷贝数据）
    gli::texture_cube cube(tex);

    // 4) 用 gli::gl 转成 OpenGL 所需格式/类型
    gli::gl translator(gli::gl::PROFILE_GL33);
    const auto glFmt   = translator.translate(tex.format(), tex.swizzles());
    const GLenum target = GL_TEXTURE_CUBE_MAP; // 目标就是 cubemap

    // 5) 绑定现有 cubemap 句柄（你的类成员）
    glBindTexture(target, this->cubemap);

    // 非对齐格式安全起见
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 6) 逐 mip / face 上传
    for (std::size_t level = 0; level < tex.levels(); ++level) {
        const auto e = tex.extent(level); // e.x, e.y
        const GLsizei w = static_cast<GLsizei>(e.x);
        const GLsizei h = static_cast<GLsizei>(e.y);

        for (std::size_t face = 0; face < 6; ++face) {
            // 通过 cube 视图拿整块图像数据
            const gli::image img = cube[face][level];
            const void* data = img.data();

            if (gli::is_compressed(tex.format())) {
                // 压缩纹理走压缩上传
                const GLsizei size = static_cast<GLsizei>(img.size());
                glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(face),
                                       static_cast<GLint>(level),
                                       glFmt.Internal, w, h, 0, size, data);
            } else {
                // 非压缩纹理走常规上传（格式/类型由 translator 给出）
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(face),
                             static_cast<GLint>(level),
                             glFmt.Internal, w, h, 0,
                             glFmt.External, glFmt.Type, data);
            }
        }
    }

    // 7) 采样与 wrap 设置（已经有 mip 的情况下，不再生成）
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, tex.levels() > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(target, 0);

    std::cout << "Successfully loaded KTX cubemap: " << path
              << " (levels=" << tex.levels() << ")\n";
}


void Cubemap::Bind(GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, this->cubemap);
}

void Cubemap::Unbind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// Create an empty cubemap
// Create mip0 - max mip levels if mipLevels > 0
void Cubemap::InitializeCubemap() {
    glGenTextures(1, &this->cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubemap);

    // Generate mip0
    for (int face = 0; face < 6; ++face) {
        for (int mip = 0; mip < this->totalMipLevels; ++mip) {
            int mipSize = this->size >> mip;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip,
                         GL_RGB32F, mipSize, mipSize, 0,
                         GL_RGB, GL_FLOAT, nullptr);
        }
    }

    // Generate mip levels if use mipmap (useMip = true) and set min filter to mimap
    if (this->mipLevels > 1) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // Generate mipmap to max levels
        
    } else {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
}

