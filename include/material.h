#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "texture/texture.h"
#include "shader.h"


class PBRMaterial {
    public:
        PBRMaterial();
        void SetAlbedoMap(const std::shared_ptr<Texture2D>& texture) { this->albedoMap = texture; };
        void SetRoughnessMap(const std::shared_ptr<Texture2D>& texture) { this->roughnessMap = texture; };
        void SetMetalnessMap(const std::shared_ptr<Texture2D>& texture) { this->metalnessMap = texture; };
        void SetNormalMap(const std::shared_ptr<Texture2D>& texture) { this->normalMap = texture; };
        void SetAOMap(const std::shared_ptr<Texture2D>& texture) { this->aoMap = texture; }; 
        void SetRoughness(float roughness) { this->roughness = roughness; };
        void SetMetalness(float metalness) { this->metalness = metalness; };
        void SetBaseColor(glm::vec3 baseColor) { this->baseColor = baseColor; };
        void SetAO(float aoFactor) { this->aoFactor = aoFactor; };
        void SetEmissive(glm::vec3 emissiveFactor) { this->emissiveFactor = emissiveFactor; };
        void LoadAlbedoMap(const std::string& path);
        void LoadRoughnessMap(const std::string& path);
        void LoadMetalnessMap(const std::string& path);
        void LoadNormalMap(const std::string& path);
        void LoadAoMap(const std::string& path);
        void LoadRoughnessMetalMap(const std::string& path); // RM map, used for loading gltf/glb
        void LoadEmissiveMap(const std::string& path);
        GLuint GetAlbedoMapTexture() const { return this->albedoMap->GetTexture(); };
        GLuint GetRoughnessMapTexture() const { return this->roughnessMap->GetTexture(); };
        GLuint GetMetalnessMapTexture() const { return this->metalnessMap->GetTexture(); };
        GLuint GetNormalMapTexture() const { return this->normalMap->GetTexture(); };
        GLuint GetAOMapTexture() const { return this->aoMap->GetTexture(); };
        GLuint GetRoughnessMetalTexture() const { return this->roughnessMetalMap->GetTexture(); }; // RM map
        GLuint GetEmissiveMapTexture() const { return this->emissiveMap->GetTexture(); };
        std::shared_ptr<Texture2D> GetAlbedoMap() const { return this->albedoMap; };
        std::shared_ptr<Texture2D> GetRoughnessMap() const { return this->roughnessMap; };
        std::shared_ptr<Texture2D> GetMetalnessMap() const { return this->metalnessMap; };
        std::shared_ptr<Texture2D> GetAOMap() const { return this->aoMap; };
        std::shared_ptr<Texture2D> GetEmissiveMap() const { return this->emissiveMap; };
        // Upload all the texture/parameters to shader
        void UploadToShader(const std::shared_ptr<Shader>& shader) const;
    private:
        std::shared_ptr<Texture2D> albedoMap            = nullptr;
        std::shared_ptr<Texture2D> roughnessMap         = nullptr;
        std::shared_ptr<Texture2D> metalnessMap         = nullptr;
        std::shared_ptr<Texture2D> normalMap            = nullptr;
        std::shared_ptr<Texture2D> aoMap                = nullptr;
        std::shared_ptr<Texture2D> roughnessMetalMap    = nullptr; // RM map, used for loading glb/gltf
        std::shared_ptr<Texture2D> emissiveMap          = nullptr;

        // Use for pure color
        glm::vec3 baseColor         = glm::vec3(1.0f, 0.0f, 0.0f);
        float roughness             = 0.5f;
        float metalness             = 0.0f;
        float aoFactor              = 0.0f;
        glm::vec3 emissiveFactor    = glm::vec3(0.0f);
};