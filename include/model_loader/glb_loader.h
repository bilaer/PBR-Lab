#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include <tiny_gltf.h>
#include "scene.h"         // SceneNode
#include "geometry.h"      // Mesh, Vertex
#include "material.h"      // PBRMaterial
#include "texture/texture.h" // Texture2D (CreateFromPixels or LoadLDRToTexture)

class GlbLoader {
public:
    // Load a GLTF/GLB file and attach created nodes under the given parent SceneNode.
    // Returns true on success.
    bool LoadFile(const std::string& path, const std::shared_ptr<SceneNode>& parent);

    // Get directory part of a path
    static std::string DirOf(const std::string& p);

private:
    // Recursively build a SceneNode from a glTF node index
    std::shared_ptr<SceneNode> BuildNodeRecursive(const tinygltf::Model& model,
                                                  int nodeIndex,
                                                  const std::string& gltfPath);

    // Build a Mesh from a single primitive (triangles/strip/fan supported; others skipped)
    std::shared_ptr<Mesh> LoadMesh(const tinygltf::Model& model,
                                   const tinygltf::Primitive& primitive);

    // Build a PBRMaterial (loads textures; sRGB/Linear respected)
    std::shared_ptr<PBRMaterial> LoadMaterial(const tinygltf::Model& model,
                                              int materialIndex,
                                              const std::string& gltfPath);

    // Access raw element pointer of an accessor at element index (handles offsets/stride)
    const unsigned char* AccessorElemPtr(const tinygltf::Model& model,
                                         const tinygltf::Accessor& acc,
                                         size_t elemIndex);

    // Derive bitangent from cross(normal, tangent) * w
    void FillBitangentsFromTangentW(std::vector<Vertex>& vertices, float defaultW = 1.0f);
};
