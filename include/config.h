#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// math
constexpr float PI = 3.14159265358979323846f;

// texture unit for PBR 
constexpr unsigned DEFAULT_TEXTURE_UNIT    = 0;
constexpr unsigned ALBEDO_TEXTURE_UNIT     = 0;
constexpr unsigned NORMAL_TEXTURE_UNIT     = 1;
constexpr unsigned ROUGHNESS_TEXTURE_UNIT  = 2;
constexpr unsigned METALNESS_TEXTURE_UNIT  = 3;
constexpr unsigned AO_TEXTURE_UNIT         = 4;
constexpr unsigned HEIGHT_TEXTURE_UNIT     = 5;
constexpr unsigned IRRADIANCE_TEXTURE_UNIT = 6;
constexpr unsigned PREFILTER_TEXTURE_UNIT  = 7;
constexpr unsigned BRDFLUT_TEXTURE_UNIT    = 8;
constexpr unsigned SKYBOX_TEXTURE_UNIT     = 9;

// Camera parameters for sampling of skybox/cubemap
constexpr glm::vec3 CAMERA_POS = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr glm::vec3 CAMERA_FRONT[6] = {
    glm::vec3( 1.0f,  0.0f,  0.0f),
    glm::vec3(-1.0f,  0.0f,  0.0f),
    glm::vec3( 0.0f,  1.0f,  0.0f),
    glm::vec3( 0.0f, -1.0f,  0.0f),
    glm::vec3( 0.0f,  0.0f,  1.0f),
    glm::vec3( 0.0f,  0.0f, -1.0f)
};

constexpr glm::vec3 CAMERA_UP[6] = {
    glm::vec3(0.0f, -1.0f,  0.0f),
    glm::vec3(0.0f, -1.0f,  0.0f),
    glm::vec3(0.0f,  0.0f,  1.0f),
    glm::vec3(0.0f,  0.0f, -1.0f),
    glm::vec3(0.0f, -1.0f,  0.0f),
    glm::vec3(0.0f, -1.0f,  0.0f)
};

const glm::mat4 CAMERA_PROJ = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);