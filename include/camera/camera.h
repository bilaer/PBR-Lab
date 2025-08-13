#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include <iostream>


enum class ProjectionMode {
    // 透视模式
    Perspective,
    // 正交模式
    Orthographic
};


class Camera {
    public:

        Camera(float fov, float aspect, float near, float far);
        virtual ~Camera() = default;


        void SetFOV(const float& fov);
        void SetAspect(const float& aspect);
        void SetNear(const float& near);
        void SetFar(const float& far);

        float GetFOV() const { return this->fov; };
        float GetNear() const { return this-> near; };
        float GetFar() const { return this->far; };
        const glm::vec3& GetFront() const { return this->cameraFront; };
        const glm::vec3& GetRight() const { return this->cameraRight; };
        const glm::mat4& GetView() const { return this->view; };
        const glm::mat4& GetProjection() const { return this->projection; };
        const glm::vec3& GetCameraPos() const { return this->cameraPosition; };

        void MoveCamera(const glm::vec3& offset);
        void RotateCamera(float yawOffset, float ptichOffset, float rollOffset);

        void UploadCameraPosition(const std::shared_ptr<Shader>& shader);
        void UploadVPMatrix(const std::shared_ptr<Shader>& shader);
        // Uplaod all camera matrix and camera position to the sahder
        void UploadToShader(const std::shared_ptr<Shader>& shader); 

        /**
        * @brief 缩放视角（FOV）
        * @param offset 视角增量（弧度）
        */
        void ZoomCamera(float offset);

        virtual void MoveForward(float delta);
        virtual void MoveBackward(float delta);
        virtual void MoveLeft(float delta);
        virtual void MoveRight(float delta);

    protected:
        // 透视模式, 默认为透视模式
        ProjectionMode projectionMode = ProjectionMode::Perspective;

        float fov;      ///< fov is in radians
        float aspect;   
        float near;     
        float far;

        // 正交参数
        float orthoLeft;
        float orthoRight;
        float orthoBottom;
        float orthoTop;

        // Euler angle (in radians)
        float yaw;      // rotate Y axis
        float pitch;    // rotate X axis
        float roll;     // rotate Z axis

        // camera matrix
        glm::mat4 view;         ///< view matrix
        glm::mat4 projection;   ///< projection matrix

        // 摄像机空间向量
        glm::vec3 cameraPosition;   ///< 摄像机当前位置
        glm::vec3 cameraUp;         ///< 摄像机“上”方向
        glm::vec3 cameraFront;      ///< 摄像机“前”方向
        glm::vec3 cameraRight;      ///< 摄像机“右”方向
        glm::vec3 worldUp;          ///< 世界空间的“上”方向（固定为Y轴）

        /**
        * @brief 根据当前状态更新视图矩阵
        */
        void UpdateViewMatrix();

        /**
         * @brief 根据 yaw/pitch 更新前向量和其他方向向量
         */
        void UpdateCameraFront();

        /**
         * @brief 限制 pitch 角度，防止万向锁
         */
        void ClampAngle();

};

// ==========================
// FirstPersonCamera 子类
// ==========================
/**
 * @brief 用第一人称游戏类摄像机控制
 */
class FirstPersonCamera: public Camera {
    public:
        FirstPersonCamera(float fov, float aspect, float near, float far);
        /**
         * @brief 前进（W 键）
         * @param delta 移动距离
         */
        void MoveForward(float delta) override;

        /**
         * @brief 后退（S 键）
         */
        void MoveRight(float delta) override;

        /**
         * @brief 向左移动（A 键）
         */
        void MoveLeft(float delta) override;

        /**
         * @brief 向右移动（D 键）
         */
        void MoveBackward(float delta) override;

};

// 第三人称类摄像头
class FixedFollowCamera {
    
};

// 星露谷风格摄像头
class TopDownCamera: public FixedFollowCamera {

};

// 马里奥风格摄像头
class SideScrollCamera: public FixedFollowCamera {

};


