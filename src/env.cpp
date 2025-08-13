#include "env.h"
#include "shader.h"
#include "config.h"
#include "cubemap/cubemap.h"

Environment::Environment(
    const std::shared_ptr<Cubemap>& irradiance, 
    const std::shared_ptr<Cubemap>& prefilter, 
    const std::shared_ptr<Texture2D>& brdflut): 
        irradiance(irradiance),
        prefilter(prefilter),
        brdflut(brdflut) {
}

Environment::Environment(): irradiance(nullptr), prefilter(nullptr), brdflut(nullptr) {

}

Environment::~Environment() {
    // Unbind all textures
    if (this->irradiance) {
        this->irradiance->Unbind();  
    }
    if (this->prefilter) {
        this->prefilter->Unbind();  
    }
    if (this->brdflut) {
        this->brdflut->Unbind();  
    }
}

// Load irradiance map from ktx file
void Environment::LoadIrradianceMap(const std::string& irradiancePath, unsigned int size) {
    this->irradiance = std::make_shared<Cubemap>(size, 0); // irradiance map only have mip0
    this->irradiance->LoadKTXToCubemap(irradiancePath);
}

// Load prefilter map from ktx file
void Environment::LoadPrefilterMap(const std::string& prefilterPath, unsigned int size, unsigned int mipLevels) {
    this->prefilter = std::make_shared<Cubemap>(size, mipLevels);
    this->prefilter->LoadKTXToCubemap(prefilterPath);

}

// Load BRDF LUT from ktx file
void Environment::LoadBRDFLut(const std::string& brdflutPath, unsigned int size) {
    this->brdflut = std::make_shared<Texture2D>(size, size, GL_RGB32F, GL_RGB, GL_FLOAT);
    this->brdflut->LoadKTXToTexture(brdflutPath);
}

void Environment::UploadToShader(const std::shared_ptr<Shader>& shader) {
    shader->Use();

    this->irradiance->Bind(IRRADIANCE_TEXTURE_UNIT);
    shader->SetUniform("irradianceMap", IRRADIANCE_TEXTURE_UNIT);

    this->prefilter->Bind(PREFILTER_TEXTURE_UNIT);
    shader->SetUniform("prefilterMap", PREFILTER_TEXTURE_UNIT);

    this->brdflut->Bind(BRDFLUT_TEXTURE_UNIT);
    shader->SetUniform("brdflut", BRDFLUT_TEXTURE_UNIT);

}