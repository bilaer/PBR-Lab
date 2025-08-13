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
        void Draw(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Camera>& camera);
        static void SetCube(std::shared_ptr<UnitCube>& cube) { Skybox::cube = cube; };
    private:
        std::shared_ptr<Cubemap> cubemap;
        static std::shared_ptr<UnitCube> cube; // static member, shared by all the skybox
        
};