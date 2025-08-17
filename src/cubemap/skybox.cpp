#include "cubemap/skybox.h"
#include "cubemap/cubemap.h"
#include "camera/camera.h"
#include "shader.h"
#include "camera/camera.h"
#include "geometry.h"
#include "config.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Initialize shared static resources
std::shared_ptr<UnitCube> Skybox::cube = nullptr;
std::shared_ptr<Shader> Skybox::skyboxShader = nullptr;

Skybox::Skybox(const std::shared_ptr<Cubemap>& cubemap): cubemap(cubemap) {

}

Skybox::Skybox(const std::string& path) {
    this->cubemap = std::make_shared<Cubemap>();
    this->cubemap->LoadEquiToCubemap(path);
}

void Skybox::Draw(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Camera>& camera) {
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    glDepthMask(GL_FALSE);

    shader->Use();
    // remove translation from view matrix
    glm::mat4 V = camera->GetView();
    glm::mat4 V_noTrans = glm::mat4(glm::mat3(V));
    shader->SetUniform("view", V_noTrans);
    shader->SetUniform("projection", camera->GetProjection());
    
    // Upload skybox texture
    shader->SetUniform("skybox", SKYBOX_TEXTURE_UNIT);
    this->cubemap->Bind(SKYBOX_TEXTURE_UNIT);

    this->cube->Draw();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);  // reset depth testing
}