#include "material.h"
#include <glm/glm.hpp>
#include "shader.h"
#include "config.h"
#include <iostream>


void PBRMaterial::BindToShader(const std::shared_ptr<Shader>& shader) const {
    shader->Use();

    // --- If use texture ---
    shader->SetUniform("useAlbedoMap",    albedoMap    ? 1 : 0);
    shader->SetUniform("useRoughnessMap", roughnessMap ? 1 : 0);
    shader->SetUniform("useMetalnessMap", metalnessMap ? 1 : 0);
    shader->SetUniform("useNormalMap",    normalMap    ? 1 : 0);
    shader->SetUniform("useAOMap",        aoMap        ? 1 : 0);

    // --- fallbacks (used when no texture is provided ) ---
    shader->SetUniform("albedo",    baseColor);
    shader->SetUniform("roughness", roughness);
    shader->SetUniform("metalness", metalness);
    shader->SetUniform("ao",        aoFactor);

    if (albedoMap) {
        albedoMap->Bind(ALBEDO_TEXTURE_UNIT);
        shader->SetUniform("albedoMap", ALBEDO_TEXTURE_UNIT);
    }
    if (roughnessMap) {
        roughnessMap->Bind(ROUGHNESS_TEXTURE_UNIT);
        shader->SetUniform("roughnessMap", ROUGHNESS_TEXTURE_UNIT);
    }
    if (metalnessMap) {
        metalnessMap->Bind(METALNESS_TEXTURE_UNIT);
        shader->SetUniform("metalnessMap", METALNESS_TEXTURE_UNIT);
    }
    if (normalMap) {
        normalMap->Bind(NORMAL_TEXTURE_UNIT);
        shader->SetUniform("normalMap", NORMAL_TEXTURE_UNIT);
    }
    if (aoMap) {
        aoMap->Bind(AO_TEXTURE_UNIT);
        shader->SetUniform("aoMap", AO_TEXTURE_UNIT);
    }
}


