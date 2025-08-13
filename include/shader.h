#pragma once
#include <string>
#include <glad/glad.h>  
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <unordered_set>

class Shader {
    public:

        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();
        void Use() const;
        GLuint GetProgramID() { return this->shaderProgram; };
        template<typename T>
        void SetUniform(const std::string& name, const T& value) const;
            
    private:
        std::string vertexPath;
        std::string fragmentPath;
        GLuint shaderProgram;
        GLuint LoadShader(const std::string& path, GLenum shaderType);
        std::string ProcessIncludes(const std::string& shaderCode, const std::string& shaderDir);
        // Use to load shader file that use include to include other file
        std::string LoadShaderWithIncludes(const std::string& path, std::unordered_set<std::string>& included);
};

template<typename T>
void Shader::SetUniform(const std::string& name, const T& value) const {
    // Get the location of the uniform variable in the shader program
    GLint location = glGetUniformLocation(this->shaderProgram, name.c_str());

    // If location == -1, the uniform does not exist or was optimized out
    if (location == -1) {
        std::cerr << "WARNING: uniform '" << name << "' not found in shader.\n";
        return;
    }

    // Check the type T at compile-time and dispatch the correct OpenGL function
    // Set integer uniform
    if constexpr (std::is_same<T, int>::value) {
        glUniform1i(location, value);  
    // Set float uniform
    } else if constexpr (std::is_same<T, float>::value) {
        glUniform1f(location, value);
    // Set bool uniform
    } else if constexpr (std::is_same<T, bool>::value) {
        glUniform1i(location, static_cast<int>(value));
    // Set vec2 uniform
    } else if constexpr (std::is_same<T, glm::vec2>::value) {
        glUniform2fv(location, 1, glm::value_ptr(value));
    // Set vec3 uniform
    } else if constexpr (std::is_same<T, glm::vec3>::value) {
        glUniform3fv(location, 1, glm::value_ptr(value));
    // Set vec4 uniform
    } else if constexpr (std::is_same<T, glm::vec4>::value) {
        glUniform4fv(location, 1, glm::value_ptr(value));
    // Set mat3 uniform
    } else if constexpr (std::is_same<T, glm::mat3>::value) {
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
    // Set mat4 uniform
    } else if constexpr (std::is_same<T, glm::mat4>::value) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    // Unsupported types trigger a compile-time error
    } else if constexpr (std::is_same<T, GLuint>::value) {
        glUniform1i(location, static_cast<int>(value));
    } else if constexpr (std::is_same<T, unsigned int>::value) {
        glUniform1i(location, static_cast<int>(value));
    } else {
        static_assert(!sizeof(T), "Unsupported uniform type");
    }
}