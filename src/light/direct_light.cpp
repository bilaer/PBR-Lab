#include "light/direct_light.h"


DirectLight::DirectLight(glm::vec3 lightDirection, glm::vec3 lightColor):
    LightBase(glm::vec3(0.0, 0.0, 0.0), lightColor),
    lightDirection(lightDirection) {
        this->type = LightType::Directed;
}
