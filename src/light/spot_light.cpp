#include "light/spot_light.h"
#include <cmath>

SpotLight::SpotLight(glm::vec3 lightPos,
                     glm::vec3 lightColorLinear,
                     glm::vec3 lightDir,
                     float intensityIBase,
                     float cosOuterHalf,
                     float cosInnerHalf,
                     float attenuationRadius)
: LightBase(lightPos, lightColorLinear, intensityIBase)
, lightDirection(glm::normalize(lightDir))
, cosOut(cosOuterHalf)
, cosIn(cosInnerHalf)
, attRadius(std::max(attenuationRadius, 1e-4f))
{
    // Enforce inner >= outer in cosine space (inner cone is narrower -> larger cosine)
    if (this->cosIn < this->cosOut) std::swap(this->cosIn, this->cosOut);

    RecalcAngleParams();
    this->invSqrAttRadius = 1.0f / (this->attRadius * this->attRadius);
}

void SpotLight::SetDirection(const glm::vec3& d) {
    this->lightDirection = glm::normalize(d);
}

void SpotLight::SetAttenuationRadius(float R) {
    this->attRadius = std::max(R, 1e-4f);
    this->invSqrAttRadius = 1.0f / (this->attRadius * this->attRadius);
}

void SpotLight::SetCosIn(float v) {
    this->cosIn = v;
    if (this->cosIn < this->cosOut) std::swap(this->cosIn, this->cosOut);
    RecalcAngleParams();
}

void SpotLight::SetCosOut(float v) {
    this->cosOut = v;
    if (this->cosIn < this->cosOut) std::swap(this->cosIn, this->cosOut);
    RecalcAngleParams();
}

void SpotLight::SetAnglesDegrees(float innerDeg, float outerDeg) {
    float ci = std::cos(glm::radians(innerDeg)); // cos(innerHalf)
    float co = std::cos(glm::radians(outerDeg)); // cos(outerHalf)
    this->cosIn  = std::max(ci, co);
    this->cosOut = std::min(ci, co);
    RecalcAngleParams();
}

void SpotLight::SetInnerDegrees(float innerDeg) {
    float ci = std::cos(glm::radians(innerDeg));
    this->cosIn = std::max(ci, this->cosOut);
    RecalcAngleParams();
}

void SpotLight::SetOuterDegrees(float outerDeg) {
    float co = std::cos(glm::radians(outerDeg));
    this->cosOut = std::min(this->cosIn, co);
    RecalcAngleParams();
}

void SpotLight::SetLumens(float lumens) {
    this->intensity = lumens / PI; // I_base
}

void SpotLight::RecalcAngleParams() {
    // Precompute scale/offset so GPU can do a single FMA + saturate:
    // t = saturate( cd * angleScale + angleOffset ), where cd = dot(lightDir, -wi)
    const float denom = std::max(0.001f, cosIn - cosOut);
    this->angleScale  = 1.0f / denom;
    this->angleOffset = -cosOut * this->angleScale;
}

void SpotLight::UploadToShader(const std::shared_ptr<Shader>& shader) {
    shader->Use();

    // Distance windowing (Frostbite-style)
    shader->SetUniform("spot.invSqrAttRadius", this->invSqrAttRadius); // 1/R^2

    // Angle attenuation precomputed params
    shader->SetUniform("spot.angleScale", this->angleScale);  // 1/(cosIn - cosOut)
    shader->SetUniform("spot.angleOffset", this->angleOffset); // -cosOut * scale

    // Core light data
    shader->SetUniform("spot.position",  this->lightPos);
    shader->SetUniform("spot.direction", this->lightDirection);   // normalized
    shader->SetUniform("spot.color",     this->lightColor);       // linear RGB
    shader->SetUniform("spot.intensity", this->intensity);        // I_base (cd)
}
