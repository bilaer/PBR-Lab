#include "camera/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Camera::Camera(float fov, float aspect, float near, float far):
    // Initialize camera at origin (0, 0, 0)
    cameraPosition(0.0f),
    // Camera pointing up
    cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw(glm::radians(90.0f)),// 初始朝向-z轴
    pitch(glm::radians(0.0f)),
    roll(glm::radians(0.0f)), //一般不使用上下翻滚
    worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), //一般情况世界坐标的上方向都是正y轴
    fov(fov), 
    aspect(aspect), 
    near(near), 
    far(far) {
        // pointing to -z axis
        this->UpdateCameraFront();
        this->UpdateViewMatrix();
        this->projection = glm::perspective(this->fov, this->aspect, this->near, this->far); 
}

// Update view matrix
void Camera::UpdateViewMatrix() {
    this->view = glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFront, this->cameraUp);
}

/**
 * @brief 根据 yaw 和 pitch 更新摄像机的前向量 cameraFront，
 * 同时更新右方向和上方向，保持正交性。
 */
void Camera::UpdateCameraFront() {
    glm::vec3 updatedCameraFront;
    updatedCameraFront.x = cos(this->yaw) * cos(this->pitch);
    updatedCameraFront.y = sin(this->pitch);
    updatedCameraFront.z = sin(this->yaw) * cos(this->pitch);
    this->cameraFront = glm::normalize(updatedCameraFront);

    // 计算摄像机右手向量，再计算真正的上向量
    // 注意世界坐标上向量不能直接当作上向量，比如当镜头抬头看天，摄像机上向量和世界坐标上向量将接近平行
    // 又因为cameraRight = ||cameraFront|||cameraRight||sin(beta) beta接近零时叉乘结果接近于零，归一化时会变成未定义或精度不可靠的向量。
    this->cameraRight = glm::normalize(glm::cross(this->cameraFront, this->worldUp));
    this->cameraUp = glm::normalize(glm::cross(this->cameraRight, this->cameraFront));
}

// Move camera position
void Camera::MoveCamera(const glm::vec3& offset) {
    this->cameraPosition += offset;
    // 摄像机前向量没有发生变化，所以直接更新view矩阵
    this->UpdateViewMatrix();
}

// Rotate camera
void Camera::RotateCamera(float yawOffset, float pitchOffset, float rollOffset) {
    this->yaw += yawOffset;
    this->pitch += pitchOffset;
    this->roll += rollOffset;
    // clamp angle to a range
    this->ClampAngle();
    // 计算摄像机前向量
    this->UpdateCameraFront();
    this->UpdateViewMatrix();
}

/** 
 * @brief 限制摄像机的 Pitch 角度在一个合理范围内，防止摄像机因为抬头/低头太多而出现“翻转”或“颠倒”现象（ gimbal lock / 万向节死锁）
 */
void Camera::ClampAngle() {
    if(this->pitch > glm::radians(89.0f)) this->pitch = glm::radians(89.0f);
    if(this->pitch < glm::radians(-89.0f)) this->pitch = glm::radians(-89.0f);
}

// Zoom in/out
void Camera::ZoomCamera(float offset) {
    this->fov += offset;

    // 限制fov角度大小
    if (this->fov < glm::radians(1.0f)) this->fov = glm::radians(1.0f);
    if (this->fov > glm::radians(90.0f)) this->fov = glm::radians(90.0f);

    //重新计算投影矩阵
    this->projection = glm::perspective(this->fov, this->aspect, this->near, this->far); 

}

// Send camera position to the shader
void Camera::UploadCameraPosition(const std::shared_ptr<Shader>& shader) {
    shader->Use();
    shader->SetUniform("viewPos", this->cameraPosition);
}

// Send view and projection matrix to shader
void Camera::UploadVPMatrix(const std::shared_ptr<Shader>& shader) {
    shader->Use();
    shader->SetUniform("view", this->view);
    shader->SetUniform("projection", this->projection);
}

void Camera::UploadToShader(const std::shared_ptr<Shader>& shader) {
    this->UploadCameraPosition(shader);
    this->UploadVPMatrix(shader);
}

void Camera::SetAspect(const float& aspect) {
    this->aspect = aspect;
    this->projection = glm::perspective(this->fov, this->aspect, this->near, this->far);
}

void Camera::SetFOV(const float& fov) {
    this->fov = fov;
    this->projection = glm::perspective(this->fov, this->aspect, this->near, this->far);
}

void Camera::SetNear(const float& near) {
    this->near = near;
    this->projection = glm::perspective(this->fov, this->aspect, this->near, this->far);
}

void Camera::SetFar(const float& far) {
    this->far = far;
    this->projection = glm::perspective(this->fov, this->aspect, this->near, this->far);
}

// ==========================
// FirstPersonCamera 
// ==========================


FirstPersonCamera::FirstPersonCamera(float fov, float aspect, float near, float far): Camera(fov, aspect, near, far) {}

// Move camera forward
void FirstPersonCamera::MoveForward(float delta) {
    this->MoveCamera(this->cameraFront * delta);
}

// Move camera backward
void FirstPersonCamera::MoveBackward(float delta) {
    this->MoveCamera(-this->cameraFront * delta);
}

// Move camera to the left
void FirstPersonCamera::MoveLeft(float delta) {
    this->MoveCamera(-this->cameraRight * delta);
}

// Move camera to the right
void FirstPersonCamera::MoveRight(float delta) {
    this->MoveCamera(this->cameraRight * delta);
}

/**
 * @brief virtual functions for movement, will be implemented by derived class
 * 
 */
void Camera::MoveForward(float) {}
void Camera::MoveBackward(float) {}
void Camera::MoveLeft(float) {}
void Camera::MoveRight(float) {}



