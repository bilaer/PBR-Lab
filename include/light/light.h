#pragma once
#include <glm/glm.hpp>

enum class LightType { Undefine = 0, Directed = 1, Point = 2, Spot = 3, Area = 4};

class LightBase {
    public:
        LightBase(glm::vec3 lightPos, glm::vec3 lightColor, float intensity = 1.0):
            lightPos(lightPos), 
            lightColor(lightColor),
            intensity(intensity) {}
        LightBase(): lightPos(glm::vec3(0.0f)), lightColor(glm::vec3(0.0f)), intensity(1.0) {};
        const glm::vec3 GetLightPos() const { return this->lightPos; };
        const glm::vec3 GetLightColor() const { return this->lightColor; };
        const float GetIntensity() const { return this->intensity; };
        void SetLightPos(glm::vec3 lightPos) { this->lightPos = lightPos; };
        void SetLightColor(glm::vec3 lightColor) { this->lightColor = lightColor; };
        void SetIntensity(float intensity) { this->intensity = intensity; };
        virtual void SetIntensityByLumen(float lumen) = 0;
        ~LightBase() = default;
    protected:
        glm::vec3 lightPos;
        glm::vec3 lightColor;
        float intensity = 1.0;
        LightType type = LightType::Undefine;
};