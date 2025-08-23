#pragma once
#include "light/light.h"
#include <glm/glm.hpp>

class DirectLight: public LightBase {
    public:
        DirectLight(glm::vec3 lightDirection, glm::vec3 lightColor);
        void SetDirection(glm::vec3 direction) { this->lightDirection = direction; };
    private:
        glm::vec3 lightDirection;
        
};