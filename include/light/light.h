/*#pragma once
#include "shader.h"
#include "object.h"
#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "materials.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>

enum LightTypeCode {
    LIGHT_TYPE_UNDEFINED = -1,
    LIGHT_TYPE_DIRECTED = 0,
    LIGHT_TYPE_POINT = 1,
    LIGHT_TYPE_SPOT = 2
};


//=========================Light============================
// Abstract class for the light object
class Light {
    public:
        Light(
            const glm::vec3& ambient, 
            const glm::vec3& diffuse, 
            const glm::vec3& specular, 
            const glm::vec3& lightColor = glm::vec3(1.0f));
        virtual ~Light() = default;
        //virtual void ShowLight(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Camera>& camera) = 0;
        int GetType() const { return this->lightTypeCode; };
        glm::vec3 GetLightColor() const { return this->lightColor; };
        glm::vec3 GetLightPos() const { return this->lightPosition; };
        int GetLightType() const { return this->lightTypeCode; };
        void SetLightColor(const glm::vec3& lightColor);
        virtual void SetLightPosition(const glm::vec3& lightPosition) = 0;
        void SetLightObj(const std::shared_ptr<Object>& lightObj) { this->lightObj = lightObj; };
        glm::vec3 GetAmbient() const { return this->ambient; };
        glm::vec3 GetDiffuse() const { return this->diffuse; };
        glm::vec3 GetSpecular() const { return this->specular; };
        void SetAmbient(glm::vec3 ambient) { this->ambient = ambient; };
        void SetDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; };
        void SetSpecular(glm::vec3 specular) { this->specular = specular; };
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader) const = 0;
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader, int index) const = 0;
        
    protected:
        int lightTypeCode;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;
        // Use to visualize the light object
        std::shared_ptr<Object> lightObj;
        //virtual void CreateEntity() = 0;
        void UploadLightIntensity(const std::shared_ptr<Shader>& shade) const;
        void UploadLightIntensity(const std::shared_ptr<Shader>& shade, int index) const;
        void UploadLightType(const std::shared_ptr<Shader>& shader) const;
        void UploadLightType(const std::shared_ptr<Shader>& shader, int index) const;
        // Create uniform name as an array
        std::string CreateUniformName(std::string valueName, int index) const;
};

//====================Directed Light=============================
class Directedlight: public Light {
    public:
        Directedlight(
            const glm::vec3& ambient,
            const glm::vec3& diffuse,
            const glm::vec3& specular,
            const glm::vec3& lightColor,
            const glm::vec3& lighgtDirection);
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader) const override;
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader, int index) const override;
        virtual void SetLightPosition(const glm::vec3& lightPosition) override {};
        glm::vec3 GetVisualizePos() const { return this->visualizePos; };
        void GenerateVisualPos();
    private:
        glm::vec3 lightDirection;
        glm::vec3 visualizePos;
};

//========================Point Light============================
class Pointlight: public Light {
    public:
        Pointlight(
            const glm::vec3& ambient,
            const glm::vec3& diffuse,
            const glm::vec3& specular,
            const glm::vec3& lightColor,
            const glm::vec3& lightPosition,
            float linear,
            float quadratic,
            float constant = 1.0);
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader) const override;
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader, int index) const override;
        virtual void SetLightPosition(const glm::vec3& lightPosition) override;
    private:
        glm::vec3 lightPosition;
        float constant;
        float linear;
        float quadratic;
};

// ====================Spot Light======================
// Can be used to make flashlight = set camera's front vector as light direction
// Cutoff is passed as angle and will be converted to cos value before passing to the shader
class Spotlight: public Light {
    public:
        Spotlight(
            const glm::vec3& ambient,
            const glm::vec3& diffuse,
            const glm::vec3& specular,
            const glm::vec3& lightColor,
            const glm::vec3& lightPosition,
            const glm::vec3& lightDirection,
            float cutoff,
            float outercutoff);
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader) const override;
        virtual void UploadToShader(const std::shared_ptr<Shader>& shader, int index) const override;
        virtual void SetLightPosition(const glm::vec3& lightPosition) override;
        void SetCutoff(float cutoff) { this->cutoff = cutoff; };
        void SetOutercutoff(float outercutoff) { this->outercutoff = outercutoff; };
        void SetlightDirection(const glm::vec3& lightDirection) { this->lightDirection = lightDirection; };
        //virtual void ShowLight(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Camera>& camera) override;
    private:
        float cutoff;
        float outercutoff;
        glm::vec3 lightDirection;
        glm::vec3 lightPosition;
        //virtual void CreateEntity() override;

};

// =================Light Manager======================
// Use to manage light and assign resources to the light
class LightManager {
    public:
        LightManager();
        void AddLight(const std::shared_ptr<Light>& light);
        void RemoveLight(unsigned int index);
        void Clear() { this->lights.clear(); };
        void UploadAllLights(std::shared_ptr<Shader>& shader);
        int GetMaxLights () const { return this->MAX_LIGHTS; };
    private:
        std::vector<std::shared_ptr<Light>> lights;
        int numLights;
        const int MAX_LIGHTS = 10; 
        std::shared_ptr<Sphere> pointlightMesh;
        std::shared_ptr<Sphere> spotlightMesh;
        std::shared_ptr<Sphere> directedlightMesh;
        std::shared_ptr<Shader> lightShader;
        // Generate mesh that are used to visualize the light entity
        void InitLightMesh();

};*/
