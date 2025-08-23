#pragma once
#include "light/light.h"
#include <glm/glm.hpp>
#include "config.h"
#include "shader.h"

class PointLight : public LightBase {
public:
    PointLight(glm::vec3 lightPos,
               glm::vec3 lightColor,
               float intensity = 1.0f,        // I (cd)
               float attenuationRadius = 10.0f);

    // Artist API: lumens (Φ) -> intensity I = Φ / (4π)
    void SetIntensityByLumen(float lumen) override { this->intensity = lumen / (4.0f * PI); }

    void SetAttenuationRadius(float R);

    void UploadToShader(const std::shared_ptr<Shader>& shader) override;

private:
    float attRadius = 10.0f;       // distance window R
    float invSqrAttRadius = 0.0f;  // 1 / (R*R)
};