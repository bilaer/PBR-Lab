#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <stdexcept>


Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath): vertexPath(vertexPath), fragmentPath(fragmentPath) {
    // Read and compile vertex and fragment shader
    GLuint vertexShader = this->LoadShader(this->vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = this->LoadShader(this->fragmentPath, GL_FRAGMENT_SHADER);

    // Link shaders into shader program
    this->shaderProgram = glCreateProgram();
    glAttachShader(this->shaderProgram, vertexShader);
    glAttachShader(this->shaderProgram, fragmentShader);
    glLinkProgram(this->shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(this->shaderProgram, 512, nullptr, infoLog);
        throw std::runtime_error("ERROR::SHADER::PROGRAM::LINKING_FAILED:\n" + std::string(infoLog));
    }

    // Clean up compiled shader objects (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

std::string Shader::ProcessIncludes(const std::string& shaderCode, const std::string& parentPath) {
    std::stringstream output;
    std::istringstream input(shaderCode);
    std::string line;

    bool versionHandled = false; // 标记是否处理过 #version

    while (std::getline(input, line)) {
        // 保留 #version 到最前
        if (!versionHandled && line.find("#version") != std::string::npos) {
            output << line << "\n";
            versionHandled = true;
            continue;
        }

        //  Process #include
        if (line.find("#include") != std::string::npos) {
            size_t start = line.find("\"") + 1;
            size_t end   = line.find("\"", start);
            std::string includeFile = line.substr(start, end - start);

            // 拼接完整路径
            std::string includePath = parentPath + "/" + includeFile;

            std::ifstream includeStream(includePath);
            if (!includeStream.is_open()) {
                throw std::runtime_error("ERROR::SHADER::INCLUDE_FILE_NOT_FOUND: " + includePath);
            }

            std::stringstream includeBuffer;
            includeBuffer << includeStream.rdbuf();

            // recursively handle include within include
            output << ProcessIncludes(includeBuffer.str(), parentPath) << "\n";
        } else {
            output << line << "\n";
        }
    }

    return output.str();
}


GLuint Shader::LoadShader(const std::string& path, GLenum shaderType) {
    std::ifstream file(path);
    if(!file.is_open()) {
        throw std::runtime_error("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " + path);
    }

    // Load shader source code
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    // Get shader folder
    std::string shaderDir = path.substr(0, path.find_last_of("/\\"));

    std::string processedCode = ProcessIncludes(buffer.str(), shaderDir);

    const char* codeCStr = processedCode.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &codeCStr, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw std::runtime_error("Shader compilation failed (" + path + "):\n" + std::string(infoLog));
    }
    return shader;
}


void Shader::Use() const {
    glUseProgram(this->shaderProgram);
}


Shader::~Shader() {
    glDeleteProgram(this->shaderProgram);
}

