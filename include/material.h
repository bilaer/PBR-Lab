#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "texture/texture.h"
#include "shader.h"


class PBRMaterial {
    public:
        enum class AlphaMode { Opaque, Mask, Blend };
        PBRMaterial();
        // -------Texture Setter--------
        void SetAlbedoMap(const std::shared_ptr<Texture2D>& texture) { this->albedoMap = texture; };
        void SetRoughnessMap(const std::shared_ptr<Texture2D>& texture) { this->roughnessMap = texture; };
        void SetMetalnessMap(const std::shared_ptr<Texture2D>& texture) { this->metalnessMap = texture; };
        void SetNormalMap(const std::shared_ptr<Texture2D>& texture) { this->normalMap = texture; };
        void SetAOMap(const std::shared_ptr<Texture2D>& texture) { this->aoMap = texture; }; 
        void SetRoughnessMetalMap(const std::shared_ptr<Texture2D>& texture) { this->roughnessMetalMap = texture; };
        void SetEmissiveMap(const std::shared_ptr<Texture2D>& texture) { this->emissiveMap = texture; };
        void SetRoughness(float roughness) { this->roughness = roughness; };
        void SetMetalness(float metalness) { this->metalness = metalness; };
        void SetBaseColor(glm::vec3 baseColor) { this->baseColor = baseColor; };
        void SetAO(float aoFactor) { this->aoFactor = aoFactor; };
        void SetEmissive(glm::vec3 emissiveFactor) { this->emissiveFactor = emissiveFactor; };
        // -----Transparent Setter------
        void SetAlphaMode(AlphaMode m) { alphaMode = m; }
        void SetAlphaCutoff(float c)   { alphaCutoff = c; }
        void SetDoubleSided(bool b)    { doubleSided = b; }
        void SetBaseAlpha(float a)     { baseAlpha = a; }
        // ------Texture Loader--------
        void LoadAlbedoMap(const std::string& path);
        void LoadRoughnessMap(const std::string& path);
        void LoadMetalnessMap(const std::string& path);
        void LoadNormalMap(const std::string& path);
        void LoadAoMap(const std::string& path);
        void LoadRoughnessMetalMap(const std::string& path); // RM map, used for loading gltf/glb
        void LoadEmissiveMap(const std::string& path);
        // -------Texture Getter-------
        GLuint GetAlbedoMapTexture() const { return this->albedoMap->GetTexture(); };
        GLuint GetRoughnessMapTexture() const { return this->roughnessMap->GetTexture(); };
        GLuint GetMetalnessMapTexture() const { return this->metalnessMap->GetTexture(); };
        GLuint GetNormalMapTexture() const { return this->normalMap->GetTexture(); };
        GLuint GetAOMapTexture() const { return this->aoMap->GetTexture(); };
        GLuint GetRoughnessMetalTexture() const { return this->roughnessMetalMap->GetTexture(); }; // RM map
        GLuint GetEmissiveMapTexture() const { return this->emissiveMap->GetTexture(); };
        float  GetAlphaCutoff() const { return alphaCutoff; }
        AlphaMode GetAlphaMode() const { return alphaMode; }
        std::shared_ptr<Texture2D> GetAlbedoMap() const { return this->albedoMap; };
        std::shared_ptr<Texture2D> GetRoughnessMap() const { return this->roughnessMap; };
        std::shared_ptr<Texture2D> GetMetalnessMap() const { return this->metalnessMap; };
        std::shared_ptr<Texture2D> GetAOMap() const { return this->aoMap; };
        std::shared_ptr<Texture2D> GetEmissiveMap() const { return this->emissiveMap; };
        // Upload all the texture/parameters to shader
        void UploadToShader(const std::shared_ptr<Shader>& shader) const;

        bool IsDoubleSided() const{ return doubleSided; }
    private:
        std::shared_ptr<Texture2D> albedoMap            = nullptr;
        std::shared_ptr<Texture2D> roughnessMap         = nullptr;
        std::shared_ptr<Texture2D> metalnessMap         = nullptr;
        std::shared_ptr<Texture2D> normalMap            = nullptr;
        std::shared_ptr<Texture2D> aoMap                = nullptr;
        std::shared_ptr<Texture2D> roughnessMetalMap    = nullptr; // RM map, used for loading glb/gltf
        std::shared_ptr<Texture2D> emissiveMap          = nullptr;

        // Fallback values when there is no textures
        glm::vec3 baseColor         = glm::vec3(1.0f, 0.0f, 0.0f);
        float roughness             = 0.5f;
        float metalness             = 0.0f;
        float aoFactor              = 0.0f;
        glm::vec3 emissiveFactor    = glm::vec3(0.0f);

        // Alpha
        AlphaMode alphaMode = AlphaMode::Opaque;
        float     alphaCutoff = 0.5f;
        bool      doubleSided = false;
        float     baseAlpha   = 1.0f;
};