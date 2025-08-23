#include "light/point_light.h"
#include <algorithm>

static constexpr float kMinRadius = 0.01f; // 1 cm, consistent with Frostbite

PointLight::PointLight(glm::vec3 lightPos,
                       glm::vec3 lightColor,
                       float intensity,
                       float attenuationRadius)
: LightBase(lightPos, lightColor, intensity)
{
    this->attRadius = std::max(attenuationRadius, kMinRadius);
    this->invSqrAttRadius = 1.0f / (this->attRadius * this->attRadius);
}

void PointLight::SetAttenuationRadius(float R) {
    this->attRadius = std::max(R, kMinRadius);
    this->invSqrAttRadius = 1.0f / (this->attRadius * this->attRadius);
}

void PointLight::UploadToShader(const std::shared_ptr<Shader>& shader) {
    shader->Use();

    // Core data (rename to your actual uniform names if different)
    shader->SetUniform("point.position",        this->lightPos);
    shader->SetUniform("point.color",           this->lightColor);     // linear RGB
    shader->SetUniform("point.intensity",       this->intensity);      // I (cd)
    shader->SetUniform("point.invSqrAttRadius", this->invSqrAttRadius);// 1/R^2
}
