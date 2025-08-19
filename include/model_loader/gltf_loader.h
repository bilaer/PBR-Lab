#pragma once
#include <memory>
#include <string>
#include <vector>
#include "tiny_gltf.h"
#include "scene.h"
#include "geometry.h"
#include "material.h"

// Vertex: { position, normals, texCoord, tangent, bitangent }
// Mesh API: SetVertices/SetIndices/SetupBuffers()
// SceneNode: SetPosition/SetRotation/SetScale/UpdateLocalTransform/UpdateWorldTransform/AddChild(...)

class GlbLoader {
public:
    // Load .glb or .gltf into your Scene (returns true on success)
    bool LoadFile(const std::string& path, const std::shared_ptr<Scene>& scene);

private:
    // Scene building
    std::shared_ptr<SceneNode> BuildNodeRecursive(const tinygltf::Model& model,
                                                  int nodeIndex,
                                                  const std::string& gltfPath);

    // Geometry
    std::shared_ptr<Mesh> LoadMesh(const tinygltf::Model& model,
                                   const tinygltf::Primitive& primitive);

    // Material & textures
    std::shared_ptr<PBRMaterial> LoadMaterial(const tinygltf::Model& model,
                                              int materialIndex,
                                              const std::string& gltfPath);

    // ---- helpers ----
    static const unsigned char* AccessorElemPtr(const tinygltf::Model& model,
                                                const tinygltf::Accessor& acc,
                                                size_t elemIndex);

    static std::string DirOf(const std::string& p);
    static std::string ResolveImagePathOrTemp(const std::string& baseDir,
                                              const tinygltf::Image& img,
                                              const std::string& hintFileName);

    static void FillBitangentsFromTangentW(std::vector<Vertex>& vertices, float defaultW = 1.0f);
};
