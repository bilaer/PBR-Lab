#pragma once
#include "tiny_gltf.h"
#include <vector>
#include <memory>
#include "scene.h"
#include "geometry.h"
#include "material.h"

class GltfLoader {
public:
    // Load a glTF file and convert it into SceneNode objects
    bool LoadFile(const std::string& path, std::shared_ptr<Scene>& scene);

private:
    // Load mesh data from glTF
    std::shared_ptr<Mesh> LoadMesh(const tinygltf::Model& model, const tinygltf::Primitive& primitive);

    // Load material data from glTF
    std::shared_ptr<PBRMaterial> LoadMaterial(const tinygltf::Model& model, int materialIndex);
};