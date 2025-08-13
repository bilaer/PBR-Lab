#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "texture/texture.h"
#include "shader.h"


class PBRMaterial {
    public:
        PBRMaterial() {};
        void SetAlbedoMap(const std::shared_ptr<Texture2D>& texture) { this->albedoMap = texture; };
        void SetRoughnessMap(const std::shared_ptr<Texture2D>& texture) { this->roughnessMap = texture; };
        void SetMetalnessMap(const std::shared_ptr<Texture2D>& texture) { this->metalnessMap = texture; };
        void SetNormalMap(const std::shared_ptr<Texture2D>& texture) { this->normalMap = texture; };
        void SetRoughness(float roughness) { this->roughness = roughness; };
        void SetMetalness(float metalness) { this->metalness = metalness; };
        void SetAO(float aoFactor) { this->aoFactor = aoFactor; };
        void BindToShader(const std::shared_ptr<Shader>& shader) const;
    private:
        std::shared_ptr<Texture2D> albedoMap        = nullptr;
        std::shared_ptr<Texture2D> roughnessMap     = nullptr;
        std::shared_ptr<Texture2D> metalnessMap     = nullptr;
        std::shared_ptr<Texture2D> normalMap        = nullptr;
        std::shared_ptr<Texture2D> aoMap            = nullptr;

        glm::vec3 baseColor = glm::vec3(1.0f, 0.0f, 0.0f);
        float roughness = 0.5f;
        float metalness = 0.0f;
        float aoFactor = 0.0f;
};