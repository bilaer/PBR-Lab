#pragma once
#include "cubemap/cubemap.h"
#include "texture/texture.h"
#include "shader.h"
#include <iostream>

// PBR environment 
class Environment {
    public:
        Environment(const std::shared_ptr<Cubemap>& irradiance, 
                    const std::shared_ptr<Cubemap>& prefilter,
                    const std::shared_ptr<Texture2D>& brdflut);
        Environment();
        ~Environment();
        void LoadIrradianceMap(const std::string& irradiancePath, unsigned int size);
        void LoadPrefilterMap(const std::string& prefilterPath, unsigned int size, unsigned int mipLevels);
        void LoadBRDFLut(const std::string& brdflutPath, unsigned int size);
        void UploadToShader(const std::shared_ptr<Shader>& shader);
        GLuint GetIrradiance() const { return this->irradiance->GetTexture(); };
        GLuint GetPrefilter() const { return this->prefilter->GetTexture(); };
        GLuint GetBRDFLUT() const { return this->brdflut->GetTexture(); };
    private:
        std::shared_ptr<Cubemap> irradiance = nullptr;    // Irradiance map
        std::shared_ptr<Cubemap> prefilter = nullptr;     // Prefilter map
        std::shared_ptr<Texture2D> brdflut = nullptr;     // BRDF LUT
};