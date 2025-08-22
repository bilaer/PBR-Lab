#include "material.h"
#include <glm/glm.hpp>
#include "shader.h"
#include "config.h"
#include <iostream>
#include <glm/glm.hpp>

PBRMaterial::PBRMaterial(): 
    baseColor(glm::vec3(1.0f, 0.0f, 0.0f)), 
    metalness(0.0f), 
    roughness(0.5f), 
    aoFactor(1.0f), 
    emissiveFactor(glm::vec3(0.0f)) {
    // Set default fallback values for material
}

void PBRMaterial::LoadAlbedoMap(const std::string& path) {
    this->albedoMap = std::make_shared<Texture2D>();
    this->albedoMap->LoadLDRToTexture(path, true); // Use sRGB format for albedo map
}

void PBRMaterial::LoadMetalnessMap(const std::string& path) {
    this->metalnessMap = std::make_shared<Texture2D>();
    this->metalnessMap->LoadLDRToTexture(path, false);
}

void PBRMaterial::LoadRoughnessMap(const std::string& path) {
    this->roughnessMap = std::make_shared<Texture2D>();
    this->roughnessMap->LoadLDRToTexture(path, false);
}

void PBRMaterial::LoadNormalMap(const std::string& path) {
    this->normalMap = std::make_shared<Texture2D>();
    this->normalMap->LoadLDRToTexture(path, false);
}

void PBRMaterial::LoadAoMap(const std::string& path) {
    this->aoMap = std::make_shared<Texture2D>();
    this->aoMap->LoadLDRToTexture(path, false);
}

void PBRMaterial::LoadRoughnessMetalMap(const std::string& path) {
    this->roughnessMetalMap = std::make_shared<Texture2D>();
    this->roughnessMetalMap->LoadLDRToTexture(path, false);
}

void PBRMaterial::LoadEmissiveMap(const std::string& path) {
    this->emissiveMap = std::make_shared<Texture2D>();
    this->emissiveMap->LoadLDRToTexture(path, true); // use sRGB format for emissive map
}

void PBRMaterial::UploadToShader(const std::shared_ptr<Shader>& shader) const {
    shader->Use();

    // --- If use texture ---
    shader->SetUniform("useRoughnessMetalMap",  roughnessMetalMap   ? 1: 0);
    shader->SetUniform("useAlbedoMap",          albedoMap           ? 1 : 0);
    shader->SetUniform("useRoughnessMap",       roughnessMap        ? 1 : 0);
    shader->SetUniform("useMetalnessMap",       metalnessMap        ? 1 : 0);
    shader->SetUniform("useNormalMap",          normalMap           ? 1 : 0);
    shader->SetUniform("useAOMap",              aoMap               ? 1 : 0);
    shader->SetUniform("useEmissiveMap",        emissiveMap         ? 1 : 0);

    // --- fallbacks (used when no texture is provided ) ---
    shader->SetUniform("baseColor", baseColor);
    shader->SetUniform("roughness", roughness);
    shader->SetUniform("metalness", metalness);
    shader->SetUniform("ao",        aoFactor);
    shader->SetUniform("emissive",  emissiveFactor);

    // --- Alpha control ---
    /*shader->SetUniform("alphaCutoff", alphaCutoff);
    shader->SetUniform("doubleSided", doubleSided);
    shader->SetUniform("alphaMode", static_cast<int>(alphaMode));*/

    // --- Normal control ---
    shader->SetUniform("useVertexTangent", this->useVertexTangent);
    shader->SetUniform("normalScale", this->normalScale);
    
    if (albedoMap) {
        shader->SetUniform("albedoMap", ALBEDO_TEXTURE_UNIT);
        albedoMap->Bind(ALBEDO_TEXTURE_UNIT);
    }
    // If use RM map or seperate roughness and metalness map
    if (roughnessMetalMap) {
        shader->SetUniform("roughnessMetalMap", ROUGHNESS_TEXTURE_UNIT);
        roughnessMetalMap->Bind(ROUGHNESS_TEXTURE_UNIT);
    } else {
        if (roughnessMap) {
            shader->SetUniform("roughnessMap", ROUGHNESS_TEXTURE_UNIT);
            roughnessMap->Bind(ROUGHNESS_TEXTURE_UNIT);
        }
        if (metalnessMap) {
            shader->SetUniform("metalnessMap", METALNESS_TEXTURE_UNIT);
            metalnessMap->Bind(METALNESS_TEXTURE_UNIT);
        }
    }

    if (normalMap) {
        shader->SetUniform("normalMap", NORMAL_TEXTURE_UNIT);
        normalMap->Bind(NORMAL_TEXTURE_UNIT);
    }

    if (aoMap) {
        shader->SetUniform("aoMap", AO_TEXTURE_UNIT);
        aoMap->Bind(AO_TEXTURE_UNIT);
    }

    if (emissiveMap) {
        shader->SetUniform("emissiveMap", EMISSIVE_TEXTURE_UNIT);
        emissiveMap->Bind(EMISSIVE_TEXTURE_UNIT);
    }

}




