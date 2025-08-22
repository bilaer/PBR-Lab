/*#include "light/light.h"

//==========================Light==================================
// abstract class for all different light
Light::Light(
    const glm::vec3& ambient, 
    const glm::vec3& diffuse, 
    const glm::vec3& specular, 
    const glm::vec3& lightColor)
    : ambient(ambient),
      diffuse(diffuse),
      specular(specular),
      lightObj(nullptr),
      lightColor(lightColor) {
        this->lightTypeCode = LIGHT_TYPE_UNDEFINED;
}

// If there is a bound light object, update its color
void Light::SetLightColor(const glm::vec3& lightColor) {
    this->lightColor = lightColor;

    if(this->lightObj) {
        auto colorMat = std::dynamic_pointer_cast<ColorMaterial>(this->lightObj->GetMaterial());
        if(colorMat) {
            colorMat->SetColor(this->lightColor);
        } else {
            std::cerr << "[ERROR] lightMaterial is not ColorMaterial\n";
        }
    }

}

// Upload ambient, diffuse, specular and color to shader
void Light::UploadLightIntensity(const std::shared_ptr<Shader>& shader) const {
    shader->Use();
    // Uplaod light parameter to the shader as [lightTyepe] + [param], for example: directedlight1_diffuse
    shader->SetUniform("light.diffuse", this->diffuse);
    shader->SetUniform("light.ambient", this->ambient);
    shader->SetUniform("light.specular", this->specular);
    shader->SetUniform("light.color", this->lightColor);
}

void Light::UploadLightIntensity(const std::shared_ptr<Shader>& shader, int index) const {
    shader->Use();
    // Uplaod light parameter to the shader as [lightTyepe] + [param], for example: directedlight1_diffuse
    shader->SetUniform(this->CreateUniformName("diffuse", index), this->diffuse);
    shader->SetUniform(this->CreateUniformName("ambient", index), this->ambient);
    shader->SetUniform(this->CreateUniformName("specular", index), this->specular);
    shader->SetUniform(this->CreateUniformName("color", index), this->lightColor);
}

// Upload basic information, such as light id and light type to the shader  
void Light::UploadLightType(const std::shared_ptr<Shader>& shader, int index) const {
    shader->Use();
    shader->SetUniform(this->CreateUniformName("lightType", index), this->lightTypeCode);
}

void Light::UploadLightType(const std::shared_ptr<Shader>& shader) const {
    shader->Use();
    shader->SetUniform("light.lightType", this->lightTypeCode);
}

std::string Light::CreateUniformName(std::string valueName, int index) const {
   std::string base = "lights[" + std::to_string(index) + "]." + valueName;
   return base;
}

//====================Directedlight=======================
Directedlight::Directedlight(
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    const glm::vec3& lightColor,
    const glm::vec3& lightDirection)
    : Light(ambient, diffuse, specular, lightColor),
      lightDirection(lightDirection) {
        this->lightTypeCode = LIGHT_TYPE_DIRECTED;
        this->GenerateVisualPos();
}


// Upload intensity and light's direction (not lightpos - fragpos)to the shader
void Directedlight::UploadToShader(const std::shared_ptr<Shader>& shader) const{
    shader->Use();
    this->UploadLightIntensity(shader);
    this->UploadLightType(shader);
    shader->SetUniform("light.direction", this->lightDirection);
}

void Directedlight::UploadToShader(const std::shared_ptr<Shader>& shader, int index) const{
    shader->Use();
    this->UploadLightIntensity(shader, index);
    this->UploadLightType(shader, index);
    shader->SetUniform(this->CreateUniformName("direction", index), this->lightDirection);
}

// Generate a point for visualizing purpose, directed light itself does not have single position
void Directedlight::GenerateVisualPos() {
    // multiple by a certain scale
    this->visualizePos = this->lightDirection * 5.0f;
}

//================Spotlight===================
Spotlight::Spotlight(
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    const glm::vec3& lightColor,
    const glm::vec3& lightPosition,
    const glm::vec3& lightDirection,
    float cutoff, 
    float outercutoff)
    : Light(ambient, diffuse, specular, lightColor),
      lightDirection(lightDirection) {
    // change angles to cos vavlue
    this->cutoff = glm::cos(glm::radians(cutoff));
    this->outercutoff = glm::cos(glm::radians(outercutoff));
    this->lightTypeCode = LIGHT_TYPE_SPOT;
}
void Spotlight::UploadToShader(const std::shared_ptr<Shader>& shader, int index) const {
    this->UploadLightIntensity(shader, index);
    this->UploadLightType(shader, index);
    shader->SetUniform(this->CreateUniformName("position", index), this->lightPosition);
    shader->SetUniform(this->CreateUniformName("direction", index), this->lightDirection);
    shader->SetUniform(this->CreateUniformName("cutoff", index), this->cutoff);
    shader->SetUniform(this->CreateUniformName("outercutoff", index), this->outercutoff);
}

void Spotlight::UploadToShader(const std::shared_ptr<Shader>& shader) const {
    this->UploadLightIntensity(shader);
    this->UploadLightType(shader);
    shader->SetUniform("light.position", this->lightPosition);
    shader->SetUniform("light.direction", this->lightDirection);
    shader->SetUniform("light.cutoff", this->cutoff);
    shader->SetUniform("light.outercutoff", this->outercutoff);
}

// If there is bound light object, update its position
void Spotlight::SetLightPosition(const glm::vec3& lightPosition) {
    this->lightPosition = lightPosition;
    if(lightObj) {
        lightObj->SetPosition(this->lightPosition);
    }
}
        
//=============Pointlight=======================
Pointlight::Pointlight(
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    const glm::vec3& lightColor,
    const glm::vec3& lightPosition,
    float linear,
    float quadratic,
    float constant)
    : Light(ambient, diffuse, specular, lightColor),
      constant(constant),
      linear(linear),
      quadratic(quadratic)
{
    this->lightPosition = lightPosition;
    this->lightTypeCode = LIGHT_TYPE_POINT;
}

void Pointlight::UploadToShader(const std::shared_ptr<Shader>& shader) const {
    shader->Use();
    this->UploadLightIntensity(shader);
    this->UploadLightType(shader);
    shader->SetUniform("light.constant", this->constant);
    shader->SetUniform("light.linear", this->linear);
    shader->SetUniform("light.quadratic", this->quadratic);
    shader->SetUniform("light.position", this->lightPosition);
}

void Pointlight::UploadToShader(const std::shared_ptr<Shader>& shader, int index) const {
    shader->Use();
    this->UploadLightIntensity(shader, index);
    this->UploadLightType(shader, index);
    shader->SetUniform(this->CreateUniformName("constant", index), this->constant);
    shader->SetUniform(this->CreateUniformName("linear", index), this->linear);
    shader->SetUniform(this->CreateUniformName("quadratic", index), this->quadratic);
    shader->SetUniform(this->CreateUniformName("position", index), this->lightPosition);
}

// If there is bound light object, update its position
void Pointlight::SetLightPosition(const glm::vec3& lightPosition) {
    this->lightPosition = lightPosition;
    if(lightObj) {
        lightObj->SetPosition(this->lightPosition);
    }
}

//=================Light Manager==================
LightManager::LightManager(): numLights(0) {
    this->InitLightMesh();
}

void LightManager::AddLight(const std::shared_ptr<Light>& light) {
    if (this->lights.size() < this->MAX_LIGHTS) {
        // Get the light color and assign to visualizing light object
        auto lightMat = std::make_shared<ColorMaterial>(light->GetLightColor());
        // Create light object based on light type
        if(light->GetLightType() == LIGHT_TYPE_DIRECTED) {
            auto lightObj = std::make_shared<Object>(this->directedlightMesh, lightMat);
            lightObj->SetPosition(std::dynamic_pointer_cast<Directedlight>(light)->GetVisualizePos());
            light->SetLightObj(lightObj);
        } else if(light->GetLightType() == LIGHT_TYPE_POINT) {
            auto lightObj = std::make_shared<Object>(this->pointlightMesh, lightMat);
            lightObj->SetPosition(light->GetLightPos());
            light->SetLightObj(lightObj);
        } else if (light->GetLightType() == LIGHT_TYPE_SPOT) {
            auto lightObj = std::make_shared<Object>(this->spotlightMesh, lightMat);
            lightObj->SetPosition(light->GetLightPos());
            light->SetLightObj(lightObj);
        } else {
            std::cerr << "ERROR::LIGHTS_MANAGER::UNDEFINED LIGHT TYPE" << std::endl;
            return;
        }
        // Set up the light id equal to index of lights vector
        this->lights.push_back(light);
        this->numLights++;
    } else {
        std::cerr << "ERROR::LIGHTS_MANAGER::EXCEED MAXIMUM LIGHTS" << std::endl;
    }
}

void LightManager::RemoveLight(unsigned int index) {
    if (index < this->lights.size()) {
        this->lights.erase(this->lights.begin() + index);
    } else {
        std::cerr << "ERROR:LIGHTS_MANAGER::INDEX OUT OF BOUND" << std::endl;
    }
}

// Upload information of all the lights to the shader
void LightManager::UploadAllLights(std::shared_ptr<Shader>& shader) {
    // check if shader is nullptr
    if (!shader) {
        std::cerr << "[ERROR] UploadAllLights called with null shader\n";
        return;
    }

    shader->Use();
    shader->SetUniform("numLights", static_cast<int>(this->lights.size()));
    for(size_t i = 0; i < this->lights.size(); ++i) {
        this->lights[i]->UploadToShader(shader, i);
    }
    
}

// Initialize the light object for visualizing
void LightManager::InitLightMesh() {
    this->pointlightMesh     = std::make_shared<Sphere>(0.1f, 20, 20);
    this->spotlightMesh      = std::make_shared<Sphere>(0.1f, 20, 20);
    this->directedlightMesh  = std::make_shared<Sphere>(0.1f, 20, 20);
} */






