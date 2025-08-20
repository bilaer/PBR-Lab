#include <iostream>
#include <glad/glad.h>

// 打印某个纹理单元上 2D/CUBE 的绑定情况
static inline void DebugDumpUnit2D(const char* tag, GLint unit) {
    GLint prev; glGetIntegerv(GL_ACTIVE_TEXTURE, &prev);
    glActiveTexture(GL_TEXTURE0 + unit);
    GLint t2d = 0, tcube = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &t2d);
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &tcube);
    std::cout << "[DBG] " << tag << " unit=" << unit
              << " 2D=" << t2d << " CUBE=" << tcube << "\n";
    glActiveTexture(prev);
}

// 打印当前 program 中某个 sampler uniform 的值（应等于你设的纹理单元）
static inline void DebugDumpSamplerUniform(const char* samplerName){
    GLint prog = 0; glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    GLint loc  = glGetUniformLocation(prog, samplerName);
    if (loc >= 0) {
        GLint val = -1; glGetUniformiv(prog, loc, &val);
        std::cout << "[DBG] " << samplerName << " uniform=" << val
                  << " (program=" << prog << ")\n";
    } else {
        std::cout << "[DBG] " << samplerName
                  << " uniform NOT FOUND in program " << prog << "\n";
    }
}