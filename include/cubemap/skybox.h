#pragma once
#include <iostream>
#include <vector>
#include "cubemap/cubemap.h"
#include "shader.h"
#include "geometry.h"
#include "camera/camera.h"

class Skybox {
    public:
        Skybox(const std::shared_ptr<Cubemap>& cubemap);
        Skybox(const std::string& path); // Load equirectangular image and convert it to cubemap
        void Draw(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Camera>& camera);
        static void SetCube(std::shared_ptr<UnitCube>& cube) { Skybox::cube = cube; };
        static void SetShader(std::shared_ptr<Shader>& shader) { Skybox::skyboxShader = shader; };
    private:
        std::shared_ptr<Cubemap> cubemap = nullptr;
        static std::shared_ptr<Shader> skyboxShader;
        static std::shared_ptr<UnitCube> cube; // static member, shared by all the skybox
        
};